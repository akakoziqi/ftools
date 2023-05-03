/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_flogs.c
 * 作者: akako
 * 修订版本: 1.0
 * 最后编辑: akako
 * 内容摘要: 分级日志输出
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

#include "utl_flogs.h"
#include "utl_ffmt.h"

// 输出颜色定义
#define LOG_COLOR_NONE "\033[0m"
#define LOG_COLOR_RED "\033[31m"
#define LOG_COLOR_GREEN "\033[32m"
#define LOG_COLOR_YELLOW "\033[33m"
#define LOG_COLOR_BLUE "\033[34m"
#define LOG_COLOR_PURPLE "\033[35m"
#define LOG_COLOR_VIOLATE "\033[36m"
#define LOG_COLOR_WHITE "\033[37m"

/// @brief 日志输出函数指针
static void (*log_output_pfun)(char* buffer, uint32_t size) = 0;

/// @brief 日志缓冲区
static char log_buffer[LOG_BUFFER_MAX_SIZE] = {0};

/// @brief 日志缓冲区剩余长度
static uint32_t buf_count = LOG_BUFFER_MAX_SIZE;

/// @brief 日志颜色等级输出前缀
static const char *log_level_prefix[] =
    {
        ""
        "\n\r" LOG_COLOR_GREEN "[SUCC] "
        "",
        ""
        "\n\r" LOG_COLOR_WHITE "[INFO] "
        "",
        ""
        "\n\r" LOG_COLOR_VIOLATE "[DEBG] "
        "",
        ""
        "\n\r" LOG_COLOR_YELLOW "[WARN] "
        "",
        ""
        "\n\r" LOG_COLOR_RED "[ERRO] "
        "",
};

/// @brief flogs 初始化
/// @param log_output_pfun 参数为 buffer 指针，长度的输出定向函数
extern void flogs_init(void (*pfun_output)(char* buffer, uint32_t size))
{
    log_output_pfun = pfun_output;
}

/// @brief 追加字符串
/// @param string 待追加的字符串指针
/// @param append 追加内容指针
/// @return 指向追加尾部的指针
static char *append2String(char *string, const char *append)
{
    uint32_t stringCnt = 0;

    while ((append[stringCnt] != '\0') &&
           (stringCnt < LOG_BUFFER_MAX_SIZE))
    {
        string[stringCnt] = append[stringCnt];
        stringCnt++;
    }
    buf_count -= stringCnt;
    return (string + stringCnt);
}

/// @brief 日志格式化输出函数
/// @param level 日志等级
/// @param tag 日志标签
/// @param fmt 格式化字符串
/// @param  
/// @return 输出字符串长度或错误值（-1）
extern int32_t flogs(uint8_t level, const char *tag, char *fmt, ...)
{
    char *pBuffer = log_buffer;
    int32_t length;
    va_list ap;

    buf_count = LOG_BUFFER_MAX_SIZE;
    pBuffer = append2String(pBuffer, log_level_prefix[level]);
    pBuffer = append2String(pBuffer, tag);
    pBuffer = append2String(pBuffer, ": ");

    va_start(ap, fmt);
    length = vsffmt(pBuffer, buf_count, fmt, &ap);
    va_end(ap);

    length += LOG_BUFFER_MAX_SIZE - buf_count;

    log_output_pfun(log_buffer, length);

    return length;
}