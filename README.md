# efsuite - embedded fast suite

目前，efsuite 以下内容：

- ffpm - 快速定点运算数学库
- ffmt - 快速格式化库
- flogs - 快速日志打印库
- fnmea - 快速 NMEA-0183 协议解析库（尚未完工）

## ffpm

ffpm 主要支持以下几种功能：

- Q12 格式定点数定义
- Q12 格式定点数四则运算
- Q12 格式定点数快速开方
- Q12 格式定点数快速对数
- Q12 格式定点数快速正余弦

## ffmt

ffmt 主要支持以下几种功能：

- 部分替代 sprintf => ffpm
- 部分替代 vsprintf => vsffpm

ffmt 语法参考：
[string] %[sign][size][type] [string]

[string]: 任意字符串

[sign]：符号说明符
[+]   输出符号位，正数为[+]，负数为[-]
[-]   隐藏符号位，正数为[空]，负数为[-]（默认）

[size]: 长度说明符

[h]		输出 8 位数据的值
[空]  输出 16 位数据的值（默认）
[l]		输出 32 位数据的值

[type]: 类型说明符

[d]		有符号十进制整数
[f]		FQ12 定点小数
[u]		无符号十进制整数
[x]		小写十六进制整数
[s]		字符串
[%]		输出 "%"

没有输出对齐与精度控制
请自行使用空格、换行与制表符控制对齐:D

## flogs
 
flogs 主要支持以下几种功能：

- 日志颜色输出
- 日志前置标签输出

flogs 的后端是 ffmt 格式化，因此支持所有 ffmt 格式化方式

## fnmea

TODO
