/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_ffmt.c
 * 作者: akako
 * 修订版本: 1.0
 * 最后编辑: akako
 * 内容摘要: 定点数快速格式化库
 * 部分参考：https://github.com/XiangYyang/G8Driver/blob/main/mcu/src/mstr/mm_fmtimp.c
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

/* 格式语法及注意事项请参考 utl_ffmt.h */

#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "utl_ffmt.h"
#include "utl_ffpm.h"

/// @brief fmt 参数存储大小 枚举
typedef enum tagFFMT_Size
{
	FFMT_Size_8,  /* 8 位 */
	FFMT_Size_16, /* 16 位 */
	FFMT_Size_32  /* 32 位 */
} FFMT_Size;

/// @brief fmt 参数符号类型 枚举
typedef enum tagFFMT_Type
{
	FFMT_Type_Signed,	/* 有符号数 */
	FFMT_Type_Unsigned, /* 无符号数 */
} FFMT_Type;

/// @brief fmt 参数输出进制 枚举
typedef enum tagFFMT_Base
{
	FFMT_Base_DEC, /* 十进制 */
	FFMT_Base_HEX, /* 十六进制 */
} FFMT_Base;

/// @brief fmt 参数输出进制 枚举
typedef enum tagFFMT_Sign
{
	FFMT_Sign_Hide, /* 隐藏符号位 */
	FFMT_Sign_Disp, /* 显示符号位 */
} FFMT_Sign;

/// @brief fmt 错误类型 枚举
typedef enum tagFFMT_Error
{
	FFMT_Error_None = 0,		  /* 无错误 */
	FFMT_Error_ArgsErr = 1 << 0,  /* 参数错误 */
	FFMT_Error_Overflow = 1 << 2, /* 缓存溢出 */
} FFMT_Error;

/// @brief fmt 当前缓存计数
static int32_t fmt_count = 0;

/// @brief fmt 当前存储大小
static FFMT_Size fmt_size = FFMT_Size_16;

/// @brief fmt 当前符号类型
static FFMT_Type fmt_type = FFMT_Type_Signed;

/// @brief fmt 当前输出进制
static FFMT_Base fmt_base = FFMT_Base_DEC;

/// @brief fmt 当前输出符号位
static FFMT_Sign fmt_sign = FFMT_Sign_Hide;

/// @brief fmt 缓冲区最大长度
static uint32_t fmt_buffer_max = 0;

/// @brief 输出符号索引
static const char fmt_index[] = "0123456789ABCDEF";

// 从 指针 取字符
#define GET_CHAR(fmt) (*(fmt))

// 移动 指针 到下一个位置
#define MOVE_TO_NEXT(fmt) ((fmt)++)

// 检查错误并返回
#define CHECK_RET(expr)             \
	{                               \
		ret |= expr;                \
		if (ret != FFMT_Error_None) \
		{                           \
			return ret;             \
		}                           \
	}

// 检查错误并跳转
#define CHECK_GOTO(expr, loc)       \
	{                               \
		ret |= expr;                \
		if (ret != FFMT_Error_None) \
		{                           \
			goto loc;               \
		}                           \
	}

/// @brief 快速模 10 函数
/// @param x 输入
/// @param div 输入 / 10
/// @param rem  输入 % 10
static inline void div_mod_10(uint32_t x, uint32_t *div, uint32_t *rem)
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

/// @brief 向缓存 push 单字符
/// @param buffer 缓存
/// @param c 待 push 单字符
/// @return FFMT_Error 枚举
static inline FFMT_Error ffmt_push_char(char *buffer, char c)
{
	if (fmt_count >= fmt_buffer_max)
	{
		/* Buffer 溢出 */
		return FFMT_Error_Overflow;
	}
	buffer[fmt_count] = c;
	fmt_count++;
	return FFMT_Error_None;
}

/// @brief fmt 完成，恢复默认状态
/// @param
static inline void ffmt_default(void)
{
	fmt_size = FFMT_Size_16;
	fmt_type = FFMT_Type_Signed;
	fmt_base = FFMT_Base_DEC;
	fmt_sign = FFMT_Sign_Hide;
}

/// @brief 无符号数 转 字符串
/// @param buffer 缓存
/// @param num 无符号数
/// @return FFMT_Error 枚举
static FFMT_Error ffmt_utos(char *buffer, uint32_t num)
{
	FFMT_Error ret = FFMT_Error_None;
	if (fmt_base == FFMT_Base_DEC)
	{
		int32_t i, j;
		uint32_t tmp;
		uint32_t rem, div;
		i = fmt_count;
		do
		{
			div_mod_10(num, &div, &rem);
			CHECK_RET(ffmt_push_char(buffer, fmt_index[rem]));
			num = div;
		} while (num);

		j = fmt_count - 1;

		// 反转字符串
		while (j - i >= 1)
		{
			tmp = buffer[j];
			buffer[j] = buffer[i];
			buffer[i] = tmp;
			i++;
			j--;
		}
	}
	if (fmt_base == FFMT_Base_HEX)
	{
		switch (fmt_size)
		{
		case FFMT_Size_32:
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 28) & 0x0000000F]));
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 24) & 0x0000000F]));
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 20) & 0x0000000F]));
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 16) & 0x0000000F]));
		case FFMT_Size_16:
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 12) & 0x0000000F]));
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 8) & 0x0000000F]));
		case FFMT_Size_8:
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 4) & 0x0000000F]));
			CHECK_RET(ffmt_push_char(buffer, fmt_index[(num >> 0) & 0x0000000F]));
		}
	}
	return ret;
}

/// @brief 有符号数 转 字符串
/// @param buffer 缓存
/// @param num 有符号数
/// @return FFMT_Error 枚举
static FFMT_Error ffmt_itos(char *buffer, int32_t num)
{
	FFMT_Error ret = FFMT_Error_None;
	if (fmt_base == FFMT_Base_DEC)
	{
		if (num < 0)
		{
			CHECK_RET(ffmt_push_char(buffer, '-'));
			CHECK_RET(ffmt_utos(buffer, (uint32_t)(-num)));
		}
		else
		{
			if (fmt_sign == FFMT_Sign_Disp)
			{
				CHECK_RET(ffmt_push_char(buffer, '+'));
			}
			CHECK_RET(ffmt_utos(buffer, (uint32_t)num));
		}
	}
	else
	{
		CHECK_RET(ffmt_utos(buffer, (uint32_t)num));
	}
	return ret;
}

/// @brief fq12_t 转 字符串
/// @param buffer 缓存
/// @param num fq12_t 类型变量
/// @return FFMT_Error 枚举
static inline FFMT_Error ffmt_fq12(char *buffer, fq12_t num)
{
	FFMT_Error ret = FFMT_Error_None;
	int32_t integer;
	int32_t decimal;
	if (num > 0)
	{
		if (fmt_sign == FFMT_Sign_Disp)
		{
			CHECK_RET(ffmt_push_char(buffer, '+'));
		}
		integer = (num >> 12);
		decimal = ((num & (0x00000FFF)) * 100000) >> 12;
	}
	else
	{
		CHECK_RET(ffmt_push_char(buffer, '-'));
		integer = (-num >> 12);
		decimal = ((-num & (0x00000FFF)) * 100000) >> 12;
	}

	CHECK_RET(ffmt_utos(buffer, integer));
	CHECK_RET(ffmt_push_char(buffer, '.'));
	if (decimal < 10)
	{
		CHECK_RET(ffmt_push_char(buffer, '0'));
	}
	if (decimal < 100)
	{
		CHECK_RET(ffmt_push_char(buffer, '0'));
	}
	if (decimal < 1000)
	{
		CHECK_RET(ffmt_push_char(buffer, '0'));
	}
	if (decimal < 10000)
	{
		CHECK_RET(ffmt_push_char(buffer, '0'));
	}
	CHECK_RET(ffmt_utos(buffer, decimal));
	return ret;
}

/// @brief ffmt 格式化函数
/// @param buffer 缓存
/// @param max_len 缓存最大长度
/// @param vs 是否传入可变参数列表
/// @param fmt 格式化字符串
/// @param
/// @return 输出字符串长度或错误值（-1）
extern int32_t ffmt(char *buffer, uint32_t max_len, bool vs, char *fmt, ...)
{
	FFMT_Error ret = FFMT_Error_None;
	va_list ap;
	va_list *ap_vs;
	fmt_count = 0;
	fmt_buffer_max = max_len;

	va_start(ap, fmt);
	// 如果上一级传入了 va_list 就替换掉
	if (vs)
	{
		ap_vs = va_arg(ap, va_list *);
	}
	else
	{
		ap_vs = &ap;
	}

	while (GET_CHAR(fmt) != '\0')
	{
		if (GET_CHAR(fmt) == '%') // 当前字符是 %
		{
			MOVE_TO_NEXT(fmt);
		ffmt_parse_sign:
			// 查找 % / s / + / -
			switch (GET_CHAR(fmt))
			{
			case '%': // 打印 %
				CHECK_GOTO(ffmt_push_char(buffer, '%'), ffmt_error_handler);
				MOVE_TO_NEXT(fmt);
				goto fmt_next_loop;
			case '+': // 显示符号位
				fmt_sign = FFMT_Sign_Disp;
				MOVE_TO_NEXT(fmt);
				goto ffmt_parse_length;
			case 's': // 字符串
				goto fmt_str;
			case '-': // 隐藏符号位
				fmt_sign = FFMT_Sign_Hide;
				MOVE_TO_NEXT(fmt);
				goto ffmt_parse_length;
			default: // 继续查找 h / l
				goto ffmt_parse_length;
			}
		ffmt_parse_length:
			// 查找  h / l
			switch (GET_CHAR(fmt))
			{
			case 'h': // 8 位
				fmt_size = FFMT_Size_8;
				MOVE_TO_NEXT(fmt);
				goto ffmt_parse_type;
			case 'l': // 32 位
				fmt_size = FFMT_Size_32;
				MOVE_TO_NEXT(fmt);
				goto ffmt_parse_type;
			default: // 继续查找 d / f / u / x
				goto ffmt_parse_type;
			}
		ffmt_parse_type:
			// 查找 d / f / u / x
			switch (GET_CHAR(fmt))
			{
			case 'd': // 有符号十进制整数
				fmt_type = FFMT_Type_Signed;
				fmt_base = FFMT_Base_DEC;
				goto fmt_int;
			case 'u': // 无符号十进制整数
				fmt_type = FFMT_Type_Unsigned;
				fmt_base = FFMT_Base_DEC;
				goto fmt_int;
			case 'x': // 小写十六进制整数
				fmt_base = FFMT_Base_HEX;
				goto fmt_int;
			case 'f': // fq12 定点数
				fmt_size = FFMT_Size_32;
				goto fmt_fq12;
			default: // 参数匹配错误，退出
				ret |= FFMT_Error_ArgsErr;
				goto ffmt_error_handler;
			}
		}
		else // 当前字符不是 %
		{
			// 正常输出
			CHECK_GOTO(ffmt_push_char(buffer, GET_CHAR(fmt)), ffmt_error_handler);
			// 移动到下一个字符
			MOVE_TO_NEXT(fmt);
			goto fmt_next_loop;
		}

	fmt_str: // 直接输出字符串
		MOVE_TO_NEXT(fmt);
		{
			char *pString = va_arg(ap, char *);
			while (GET_CHAR(pString) != '\0')
			{
				CHECK_GOTO(ffmt_push_char(buffer, GET_CHAR(pString)), ffmt_error_handler);
				MOVE_TO_NEXT(pString);
			}
		}
		goto fmt_finish;

	fmt_int: // 格式化整数
		MOVE_TO_NEXT(fmt);
		{
			if (fmt_type == FFMT_Type_Signed)
			{
				
				CHECK_GOTO(ffmt_itos(buffer, va_arg(*ap_vs, int32_t)), ffmt_error_handler);
			}
			if (fmt_type == FFMT_Type_Unsigned)
			{
				CHECK_GOTO(ffmt_utos(buffer, va_arg(*ap_vs, uint32_t)), ffmt_error_handler);
			}
		}
		goto fmt_finish;

	fmt_fq12: // 格式化
		MOVE_TO_NEXT(fmt);
		CHECK_GOTO(ffmt_fq12(buffer, va_arg(*ap_vs, fq12_t)), ffmt_error_handler);
		goto fmt_finish;

	fmt_finish: // 完成一次格式化，重置参数
		ffmt_default();

	fmt_next_loop:
		continue;
	}
	// 正常字符串结束
	CHECK_GOTO(ffmt_push_char(buffer, '\0'), ffmt_error_handler);

ffmt_error_handler:
	va_end(ap);
	// 没问题正常返回
	if (ret == FFMT_Error_None)
	{
		return fmt_count;
	}
	// 寄了，尝试强制结束字符串
	ffmt_push_char(buffer, '\0');
	buffer[fmt_buffer_max - 1] = '\0';
	return -1;
}

/// @brief ffmt 格式化函数
/// @param buffer 缓存
/// @param max_len 缓存最大长度
/// @param fmt 格式化字符串
/// @param agrs
/// @return 输出字符串长度或错误值（-1）
extern int32_t vsffmt(char *buffer, uint32_t max_len, char *fmt, va_list *agrs)
{
	return ffmt(buffer, max_len, true, fmt, agrs);
}
