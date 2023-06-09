/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 文件名: utl_fnmea.h
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
 *
 * 本 NMEA 协议解析库支持以下协议的解析：
 *
 * --GGA GPS 定位信息（UTC 时间、经纬度、定位状态、使用卫星数量、精度、海拔等）
 * --GSA 当前卫星信息（模式、定位类型、PRN 码、精度因子等）
 * --GSV 可见卫星信息（可见卫星数、PRN 码、卫星仰角、卫星方位角、信噪比等）
 * --RMC 推荐定位信息（UTC 日期时间、定位状态、经纬度、速度方向、磁偏角等）
 * --VTG 地面速度信息（地面航向、地面速率等）
 * --GLL 定位地理信息（UTC 时间、经纬度、定位状态）
 *
 * 其中可根据需要自定义配置单独解析哪种指令
 *
 * 可供使用的信息有：
 *
 * 1. UTC 日期
 * 2. UTC 时间
 * 3. 定位状态
 * 4. 定位信息（经纬度、海拔高度）
 * 5. 地面速度（kph、sog）
 * 6. 地面航向（正北方向角、磁北方向角）
 * 7. 定位精度（精度因子、可见卫星数）
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef __UTL_FNMEA_H__
#define __UTL_FNMEA_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#define FNMEA_BUFFER_MAX_SIZE 256

/// @brief 定位状态定义
typedef enum tagFNMEA_Status
{
	FNMEA_STAT_Autonomous,	 // 自主模式（A）
	FNMEA_STAT_Estimation,	 // 估算模式（E）
	FNMEA_STAT_Invalid,		 // 数据无效（N）
	FNMEA_STAT_Differential, // 差分模式（D）
} fnmea_status_e;

/// @brief 定位类型定义
typedef enum tagFNMEA_Type
{
	FNMEA_TYPE_BD,	 // 北斗导航卫星系统（BDS）
	FNMEA_TYPE_GP,	 // 全球定位系统（GPS、SBAS、QZSS） 
	FNMEA_TYPE_GL,		 // 格洛纳斯定位系统（GLONASS） 
	FNMEA_TYPE_GN, // 全球导航卫星系统（GNSS） 
} fnmea_type_e;

/// @brief UTC 时间定义
typedef struct tagFNMEA_UTC_Time
{
	uint8_t hour; // 小时
	uint8_t min;  // 分钟
	uint8_t sec;  // 秒
	uint16_t msec; // 毫秒
} fnmea_utc_time_t;

/// @brief UTC 日期定义
typedef struct tagFNMEA_UTC_Date
{
	uint8_t mon;   // 月
	uint8_t day;   // 日
	uint16_t year; // 年
} fnmea_utc_date_t;

/// @brief 经度信息定义
typedef struct tagFNMEA_Degree
{
	uint8_t sign;// 东西经，东经为 0，西经为非 0（通常为 1）
				 // 南北纬，南纬为 0，北纬为非 0（通常为 1）
	uint8_t deg; // 度
	uint8_t min; // 分(整数部分)
	uint16_t min_dec; // 分(小数部分)
} fnmea_degree_t;

/// @brief 位置信息（经纬度）定义
typedef struct tagFNMEA_Location
{
	fnmea_degree_t longi; // 经度
	fnmea_degree_t lati;	 // 纬度
	int32_t alti;			 // Q12 海拔
} fnmea_location_t;

/// @brief 对地速度信息定义
typedef struct tagFNMEA_Speed
{
	int32_t speed_kph; // Q12 速度单位：千米每小时
	int32_t speed_sog; // Q12 速度单位：节
} fnmea_speed_t;

/// @brief 对地航向信息定义
typedef struct tagFNMEA_Angle
{
	int32_t true_north;		// Q12 正北方向角
	int32_t magnetic_north; // Q12 磁北方向角
} fnmea_angle_t;

typedef struct tagFNMEA_Precision
{
	uint8_t satellite_num; // 可用卫星数量
	uint8_t pdop;		   // 位置精度因子
	uint8_t hdop;		   // 水平精度因子
	uint8_t vdpop;		   // 垂直精度因子
} fnmea_precision;

typedef struct tagFNEMA
{
	fnmea_type_e type;
	fnmea_status_e staus;
	fnmea_utc_time_t utc_time;
	fnmea_utc_date_t utc_date;
	fnmea_location_t locate;
	fnmea_speed_t speed;
	fnmea_angle_t angle;
	fnmea_precision precision;
} fnmea_t;

#endif
