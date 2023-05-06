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

#define INDEX_BUFFER() (nmea_buffer[nmea_buffer_cnt])
#define MOVE_TO_NEXT() (nmea_buffer_cnt++)
#define CHECK_RETURN(expr) {ret=expr;if(ret!=FNMEA_Error_None){return ret;}}

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
	FNMEA_Error_Parse_uint8,
	FNMEA_Error_Parse_uint16,
	FNMEA_Error_Parse_int32,
	FNMEA_Error_Parse_type,
	FNMEA_Error_Parse_CMD_type,

} FNMEA_Error_Type;

static FNMEA_CMD_Type nmea_cmd_type;
static fnmea_type_e nmea_type;
static fnmea_status_e nmea_staus;
static fnmea_utc_time_t nmea_time;
static fnmea_utc_date_t nmea_date;
static fnmea_location_t nmea_locate;
static fnmea_speed_t nmea_speed;
static fnmea_angle_t nmea_angle;
static fnmea_precision nmea_preci;

/// @brief NMEA 指令 Buffer
static char nmea_buffer[FNMEA_BUFFER_MAX_SUZE] = "";
uint32_t nmea_buffer_cnt = 0;

/// 
static FNMEA_Error_Type f_parse_uint8(uint8_t* pNum)
{
	uint8_t num = 0;
	while (INDEX_BUFFER()!=',')
	{
		if (!(INDEX_BUFFER() >= '0' && INDEX_BUFFER() <= '9'))
		{
			return FNMEA_Error_Parse_uint8;
		}
		num = num * 10 + (GET_CHAR() - '0');
		MOVE_TO_NEXT();
	}
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type f_parse_uint16(uint16_t* pNum)
{
	uint16_t num = 0;
	while (INDEX_BUFFER() != ',')
	{
		if (!(INDEX_BUFFER() >= '0' && INDEX_BUFFER() <= '9'))
		{
			return FNMEA_Error_Parse_uint16;
		}
		num = num * 10 + (GET_CHAR() - '0');
		MOVE_TO_NEXT();
	}
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type f_parse_int32(int32_t* pNum)
{
	int32_t num = 0;
	int8_t sign = 1;
	while (INDEX_BUFFER() != ',')
	{
		if (!((INDEX_BUFFER() == '-') || (INDEX_BUFFER() >= '0' && INDEX_BUFFER() <= '9')))
		{
			return FNMEA_Error_Parse_int32;
		}
		if (INDEX_BUFFER() == '-')
		{
			sign = -1;
			MOVE_TO_NEXT();
			continue;
		}
		num = num * 10 + (INDEX_BUFFER() - '0');
		MOVE_TO_NEXT();
	}
	num *= sign;
	*pNum = num;
	return FNMEA_Error_None;
}

/// 
static FNMEA_Error_Type f_parse_nmea_type(fnmea_type_e* type)
{
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'G':
		goto parse_nmea_type_stage_2;
	case 'B':
		goto parse_nmea_type_stage_3;
	default:
		return FNMEA_Error_Parse_type;
	}
parse_nmea_type_stage_2:
	switch (INDEX_BUFFER())
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
parse_nmea_type_stage_3:
	if (INDEX_BUFFER() == 'D')
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
static FNMEA_Error_Type f_parse_nmea_cmd_type(FNMEA_CMD_Type* cmd)
{
	// 第一个字符
	switch (INDEX_BUFFER())
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
	switch (INDEX_BUFFER())
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
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'M':
		goto parse_nmea_cmd_type_stage_1r2m;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1v:
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'T':
		goto parse_nmea_cmd_type_stage_1v2t;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
	// 第三个字符
parse_nmea_cmd_type_stage_1g2g:
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'A':
		nmea_cmd_type = FNMEA_CMD_GGA;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1g2l:
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'L':
		nmea_cmd_type = FNMEA_CMD_GLL;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1g2s:
	switch (INDEX_BUFFER())
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
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'C':
		nmea_cmd_type = FNMEA_CMD_RMC;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
parse_nmea_cmd_type_stage_1v2t:
	switch (INDEX_BUFFER())
	{
		MOVE_TO_NEXT();
	case 'G':
		nmea_cmd_type = FNMEA_CMD_VTG;
		return FNMEA_Error_None;
	default:
		return FNMEA_Error_Parse_CMD_type;
	}
}


static FNMEA_Error_Type f_parse_nmea_utc_time(fnmea_utc_time_t* time)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	CHECK_RETURN(f_parse_uint8(&time->hour));
	CHECK_RETURN(f_parse_uint8(&time->min));
	CHECK_RETURN(f_parse_uint8(&time->sec));
	CHECK_RETURN(f_parse_uint8(&time->msec));

}

int main(void)
{
	FNMEA_Error_Type ret = FNMEA_Error_None;
	uint32_t num = 0;

	ret = f_parse_int32(&num);

	return 0;
}