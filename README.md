# ftools 套件

目前，fast tools 以下内容：

- ffpm - 快速定点运算数学库
- ffmt - 快速格式化库
- flogs - 快速日志打印库

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

具体细节参考 utl_ffmt.h

## flogs
 
ffpm 主要支持以下几种功能：

- 日志颜色输出
- 日志前置标签输出

