# LwCLI
---------------------
LwCLI是一个命令行交互界面，完全采用C语言编写，特点是轻量、资源占用少，非常适合嵌入式设备。LwCLI设计时充分考虑了移植性问题，对接口封装设计比较好，因此非常容易移植到不同的平台上。

## 功能介绍
LwCLI的主要功能：
* 支持用户登录/登出功能。
* 支持历史命令记录/查询。
* 支持backspace。
* 支持提示语设置。
* 内置help命令。

## 文件介绍

### cli_line.c/h
实现提示语、退格、行解析、历史命令记录/查询。本模块无任何依赖，可单独作为库使用。

### cli_command.c/h
提供命令注册接口，支持help查询，复制解析命令行，并调用相关命令处理函数。本模块无任何依赖，可单独作为库使用。

### cli_shell.c/h
依赖cli_line和cli_command，创建line和command对象，并进一步接口封装，内置登录/登出功能，对外提供相关API接口。

## demo演示
编译和运行环境：ubuntu 16

### 编译
cd /demo/linux
make

### 运行
./build/lwcli_demo

### 登录
Login: admin
Password: admin

## API接口介绍
* CliShellInit：初始化lwcli，调用这需要指定最大支持的命令数，还需要指定输出接口(可以是printf，也可以是其它函数)
* CliShellRegister：命令行注册函数
* CliShellInputChar：输入接口，输入单个字符给lwcli
* CliShellInputBlock：输入接口，输入块数据给lwcli
* CliShellTick：需要周期调用，用于命令行的解析和处理




