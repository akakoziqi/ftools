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
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#include "utl_fnmea.h"

static fnmea_status_e nmea_staus;
static fnmea_utc_time_t nmea_time;
static fnmea_utc_date_t nmea_date;
static fnmea_location_t nmea_locate;
static fnmea_speed_t nmea_speed;
static fnmea_angle_t nmea_angle;
static fnmea_precision nmea_preci;

