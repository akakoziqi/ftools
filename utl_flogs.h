/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_flogs.h
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

#ifndef __FLOGS_H__
#define __FLOGS_H__

#include <stdint.h>

// 日志最大缓冲区
#define LOG_BUFFER_MAX_SIZE (256) // Bytes
// 最大输出日志等级
#define FLOG_LEVEL LEVEL_SUCCESS

// 日志等级定义
#define   LEVEL_SUCCESS  0
#define   LEVEL_INFO  1
#define   LEVEL_DEBUG  2
#define   LEVEL_WARN  3
#define   LEVEL_ERROR  4
#define   LEVEL_NONE  5

#if (FLOG_LEVEL <= LEVEL_SUCCESS)
#define FLOGS(tag, ...) flogs(LEVEL_SUCCESS, tag, __VA_ARGS__)
#else
#define FLOGS(tag, ...)
#endif

#if (FLOG_LEVEL <= LEVEL_INFO)
#define FLOGI(tag, ...) flogs(LEVEL_INFO, tag, __VA_ARGS__)
#else
#define FLOGI(tag, ...)
#endif

#if (FLOG_LEVEL <= LEVEL_DEBUG)
#define FLOGD(tag, ...) flogs(LEVEL_DEBUG, tag, __VA_ARGS__)
#else
#define FLOGD(tag, ...)
#endif

#if (FLOG_LEVEL <= LEVEL_WARN)
#define FLOGW(tag, ...) flogs(LEVEL_WARN, tag, __VA_ARGS__)
#else
#define FLOGW(tag, ...)
#endif

#if (FLOG_LEVEL <= LEVEL_ERROR)
#define FLOGE(tag, ...) flogs(LEVEL_ERROR, tag, __VA_ARGS__)
#else
#define FLOGE(tag, ...)
#endif

extern void flogs_init(void (*pfun_output)(char* buffer, uint32_t size));
extern int32_t flogs(uint8_t level, const char *tag, char *fmt, ...);

#endif
