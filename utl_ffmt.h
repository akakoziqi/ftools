/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_ffmt.h
 * 作者: akako
 * 修订版本: 1.0
 * 最后编辑: akako
 * 内容摘要: 定点数快速格式化库
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
  * FFMT 格式化规则：
  *
  * [string] %[size][type] [string]
  *
  * [string]: 任意字符串
  *
  * [size]: 长度说明符
  *
  * [h]		输出 8 位数据的值
  * [空]  输出 16 位数据的值（默认）
  * [l]		输出 32 位数据的值
  *
  * [type]: 类型说明符
  *
  * [d]		有符号十进制整数
  * [f]		FQ12 定点小数
  * [u]		无符号十进制整数
  * [x]		小写十六进制整数
  * [s]		字符串
  * [%]		输出 "%"
  *
  * 本 fmt 是 printf 的极致优化（残废）版本
  * 为性能低的嵌入式系统设计
  * 部分语法与 printf 不相同，请一定一定一定注意
  * 干掉也许没什么用的语法，您的 intellisense 不会报错，但运行会炸
  * 统一正数输出加号(+)，负数输出减号(-)
  * 没有输出对齐与精度控制
  * 自行使用空格、换行与制表符控制对齐:D
  *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __UTL_FFMT_H__
#define __UTL_FFMT_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

extern int32_t ffmt(char *buffer, uint32_t max_len, bool vs, char *fmt, ...);
extern int32_t vsffmt(char *buffer, uint32_t max_len, char *fmt, va_list *agrs);

#endif
