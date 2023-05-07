/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_fnmea.c
 * 作者: akako
 * 修订版本: 1.0
 * 最后编辑: akako
 * 内容摘要: 快速 NMEA 协议解析库
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Email: akako.ziqi@outlook.com
 *
 * Copyright (C) 2023 akako
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * 本程序仅供参考，使用本程序造成的一切后果与作者无关，由您自己负责。
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * NMEA-0183 协议简述
 * 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#include "utl_fnmea.h"

#define INDEX_BUFFER(buf) (buf[nmea_buffer_cnt])
#define MOVE_TO_NEXT() {if(nmea_buffer_cnt<=FNMEA_BUFFER_MAX_SIZE)nmea_buffer_cnt++;else{return FNMEA_Error_Overflow;}}
#define CHECK_RETURN(expr) {ret=expr;if(ret!=FNMEA_Error_None){return ret;}}
#define MOVE_TO_HEAD() (nmea_buffer_cnt=0) 
#define MOVE_TO_NEXT_PARAM(buf) {while(INDEX_BUFFER(buf)!=','){MOVE_TO_NEXT();}MOVE_TO_NEXT();}

typedef enum tagFNMEA_CMD_Type
{
	FNMEA_CMD_GGA,
	FNMEA_CMD_GSA,
	FNMEA_CMD_GSV,
	FNMEA_CMD_RMC,
	FNMEA_CMD_VTG,
	FNMEA_CMD_GLL
} FNMEA_CMD_Type;

typedef enum tagFNMEA_Error_Type
{
	FNMEA_Error_None,
	FNMEA_Error_Overflow,
	FNMEA_Error_Parse_uint8,
	FNMEA_Error_Parse_uint16,
	FNMEA_Error_Parse_int32,
	FNMEA_Error_Parse_type,
	FNMEA_Error_Parse_CMD_type,
	FNMEA_Error_Parse_Fix_Char

} FNMEA_Error_Type;

static FNMEA_CMD_Type nmea_cmd_type;

static fnmea_t fnema;

/// @brief NMEA Buffer 计数
uint32_t nmea_buffer_cnt = 0;

/// @brief 快速模 10 函数
/// @param x 输入
/// @param div 输入 / 10
/// @param rem  输入 % 10
static void div_mod_10(uint32_t x, uint32_t* div, uint32_t* rem)
{
	uint32_t q = (x >> 1) + (x >> 2);
	q += (q >> 4);
	q += (q >> 8);
	q += (q >> 16);
	// q = x * 0.8, 现在计算q / 8, 得到x * 0.1
	q >>= 3;
	// 计算结果
	uint32_t r = x - (q * 10);
	if (r > 9)
	{
		*div = q + 1;
		*rem = r - 10;
	}
	else
	{
		*div = q;
		*rem = r;
	}
}

/// @brief 快速模 100 函数
/// @param x 输入
/// @param div 输入 / 100
/// @param rem  输入 % 100
static void div_mod_100(uint32_t x, uint32_t* div, uint32_t* rem)
{
	uint32_t q = (x >> 3) + (x >> 5) + (x >> 9) + (x >> 10) + (x >> 11) + (x >> 12);
	// q = x * 0.16, 现在计算q / 16, 得到x * 0.01
	q >>= 4;
	// 计算结果
	uint32_t r = x - (q * 100);
	if (r > 199)
	{
		*div = q + 2;
		*rem = r - 200;
	}
	else if (r > 99)
	{
		*div = q + 1;
		*rem = r - 100;
	}
	else
	{
		*div = q;
		*rem = r;
	}
}

/// 
static FNMEA_Error_Type fnmea_parse_uint8(char* pbuffer, uint8_t* pNum)
{
	uint8_t num = 0;
	while (INDEX_BUFFER(pbuffer) != ',' && INDEX_BUFFER(pbuffer) != '.' && INDEX_BUFFER(pbuffer) != '\0')
	{
		if (!(INDEX_BUFFER(pbuffer) >= '0' && INDEX_BUFFER(pbuffer) <= '9'))
		{
			return FNMEA_Error_Parse_uint8;
		}
		num = num * 10 + (INDEX_BUFFER(pbuffer) - '0');
		MOVE_TO_NEXT();
	}
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type fnmea_parse_uint16(char* pbuffer, uint16_t* pNum)
{
	uint16_t num = 0;
	while (INDEX_BUFFER(pbuffer) != ',' && INDEX_BUFFER(pbuffer) != '.' && INDEX_BUFFER(pbuffer) != '\0')
	{
		if (!(INDEX_BUFFER(pbuffer) >= '0' && INDEX_BUFFER(pbuffer) <= '9'))
		{
			return FNMEA_Error_Parse_uint16;
		}
		num = num * 10 + (INDEX_BUFFER(pbuffer) - '0');
		MOVE_TO_NEXT();
	}
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type fnmea_parse_int32(char* pbuffer, int32_t* pNum)
{
	int32_t num = 0;
	int8_t sign = 1;
	while (INDEX_BUFFER(pbuffer) != ',' && INDEX_BUFFER(pbuffer) != '.' && INDEX_BUFFER(pbuffer) != '\0')
	{
		if (!((INDEX_BUFFER(pbuffer) == '-') || (INDEX_BUFFER(pbuffer) >= '0' && INDEX_BUFFER(pbuffer) <= '9')))
		{
			return FNMEA_Error_Parse_int32;
		}
		if (INDEX_BUFFER(pbuffer) == '-')
		{
			sign = -1;
			MOVE_TO_NEXT();
			continue;
		}
		num = num * 10 + (INDEX_BUFFER(pbuffer) - '0');
		MOVE_TO_NEXT();
	}
	num *= sign;
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type fnmea_parse_type(char* pbuffer, fnmea_type_e* type)
{
	// 第一个字符
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'G':
		goto parse_nmea_type_stage_1g;
	case 'B':
		goto parse_nmea_type_stage_1b;
	default:
		return FNMEA_Error_Parse_type;
	}
	// 第二个字符
parse_nmea_type_stage_1g:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'P':
		*type = FNMEA_TYPE_GP;
		return FNMEA_Error_None;
	case 'L':
		*type = FNMEA_TYPE_GL;
		return FNMEA_Error_None;
	case 'N':
		*type = FNMEA_TYPE_GN;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_type;
	}
parse_nmea_type_stage_1b:
	if (INDEX_BUFFER(pbuffer) == 'D')
	{
		MOVE_TO_NEXT();
		*type = FNMEA_TYPE_BD;
		return FNMEA_Error_None;
	}
	else
	{
		MOVE_TO_NEXT();
		return FNMEA_Error_Parse_type;
	}
}

/// 
static FNMEA_Error_Type fnmea_parse_cmd_type(char* pbuffer, FNMEA_CMD_Type* cmd)
{
	// 第一个字符
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'G':
		goto parse_nmea_cmd_type_stage_1g;
	case 'R':
		goto parse_nmea_cmd_type_stage_1r;
	case 'V':
		goto parse_nmea_cmd_type_stage_1v;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
	// 第二个字符
parse_nmea_cmd_type_stage_1g:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'G':
		goto parse_nmea_cmd_type_stage_1g2g;
	case 'L':
		goto parse_nmea_cmd_type_stage_1g2l;
	case 'S':
		goto parse_nmea_cmd_type_stage_1g2s;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1r:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'M':
		goto parse_nmea_cmd_type_stage_1r2m;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1v:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'T':
		goto parse_nmea_cmd_type_stage_1v2t;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
	// 第三个字符
parse_nmea_cmd_type_stage_1g2g:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'A':
		nmea_cmd_type = FNMEA_CMD_GGA;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1g2l:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'L':
		nmea_cmd_type = FNMEA_CMD_GLL;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1g2s:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'A':
		nmea_cmd_type = FNMEA_CMD_GSA;
		return FNMEA_Error_None;
	case 'V':
		nmea_cmd_type = FNMEA_CMD_GSV;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1r2m:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'C':
		nmea_cmd_type = FNMEA_CMD_RMC;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1v2t:
	switch (INDEX_BUFFER(pbuffer))
	{
		MOVE_TO_NEXT();
	case 'G':
		nmea_cmd_type = FNMEA_CMD_VTG;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
}

static FNMEA_Error_Type fnmea_parse_fix_char(char* pbuffer, char fix)
{
	if (INDEX_BUFFER(pbuffer) == fix)
	{
		MOVE_TO_NEXT();
		return FNMEA_Error_None;
	}
	else
	{
		return FNMEA_Error_Parse_Fix_Char;
	}
}

static FNMEA_Error_Type fnmea_parse_utc_time(char* pbuffer, fnmea_utc_time_t* time)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	int32_t time_num;
	uint32_t div;
	uint32_t rem;

	// 获取时间信息
	CHECK_RETURN(fnmea_parse_int32(pbuffer, &time_num));
	// 解析秒
	div_mod_100((uint32_t)time_num, &div, &rem);
	time->sec = rem;
	// 解析分和时
	div_mod_100(div, &div, &rem);
	time->min = rem;
	time->hour = div;
	// 解析小数点
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, '.'));
	// 解析毫秒
	CHECK_RETURN(fnmea_parse_uint16(pbuffer, &time->msec));

	return ret;
}

static FNMEA_Error_Type fnmea_parse_degree(char* pbuffer, fnmea_degree_t* degree)
{
	FNMEA_Error_Type ret;
	uint16_t integer;
	uint16_t decimal;

	uint32_t div;
	uint32_t rem;

	// 格式为：(d)ddmm.mmmm
	CHECK_RETURN(fnmea_parse_uint16(pbuffer, &integer)); // (d)ddmm
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, '.'));// .
	CHECK_RETURN(fnmea_parse_uint16(pbuffer, &decimal));// mmmm

	// 解析：(d)dd
	div_mod_100((uint32_t)integer, &div, &rem);
	// 解析：mm.mmmm
	degree->deg = (uint8_t)div;
	degree->min = (uint8_t)rem;
	degree->min_dec = decimal;

	return ret;
}

static FNMEA_Error_Type fnmea_parse_location(char* pbuffer, fnmea_location_t* locate)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	// 纬度数值
	CHECK_RETURN(fnmea_parse_degree(pbuffer, &locate->lati));
	// ,
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, ','));
	// 纬度符号
	if (INDEX_BUFFER(pbuffer) == 'N')
		locate->lati.sign = 1;
	else if (INDEX_BUFFER(pbuffer) == 'S')
		locate->lati.sign = 0;
	else
		return FNMEA_Error_Parse_Fix_Char;

	MOVE_TO_NEXT();
	// ,
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, ','));
	// 经度数值
	CHECK_RETURN(fnmea_parse_degree(pbuffer, &locate->longi));
	// ,
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, ','));
	// 经度符号
	if (INDEX_BUFFER(pbuffer) == 'E')
		locate->longi.sign = 0;
	else if (INDEX_BUFFER(pbuffer) == 'W')
		locate->longi.sign = 1;
	else
		return FNMEA_Error_Parse_Fix_Char;

	MOVE_TO_NEXT();
	// ,
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, ','));
}




static FNMEA_Error_Type fnmea_parse_cmd_GCA(char* pbuffer, fnmea_t* nmea)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	CHECK_RETURN(fnmea_parse_fix_char(pbuffer, '$'));
	CHECK_RETURN(fnmea_parse_utc_time(pbuffer, &nmea->utc_time));
	CHECK_RETURN(fnmea_parse_location(pbuffer, &nmea->locate));
	MOVE_TO_NEXT_PARAM(pbuffer);


}

char utc_time_test_buffer[FNMEA_BUFFER_MAX_SIZE] = "235316.000";
char degree_test_buffer[FNMEA_BUFFER_MAX_SIZE] = "12000.0090";
char location_test_buffer[FNMEA_BUFFER_MAX_SIZE] = "2959.9925,N,12000.0090,W,";


int main(void)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	fnmea_utc_time_t time;
	fnmea_degree_t degree;
	fnmea_location_t locate;

	ret = fnmea_parse_utc_time(utc_time_test_buffer, &time);
	MOVE_TO_HEAD();
	ret = fnmea_parse_degree(degree_test_buffer, &degree);
	MOVE_TO_HEAD();
	ret = fnmea_parse_location(location_test_buffer, &locate);
	MOVE_TO_HEAD();

	return 0;
}