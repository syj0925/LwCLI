# LwCLI
---------------------
LwCLI是一个命令行交互界面，完全采用C语言编写，特点是轻量、资源占用少，非常适合嵌入式设备。LwCLI设计时充分考虑了移植性问题，对接口封装设计比较好，因此非常容易移植到不同的平台上。

## 功能介绍
LwCLI的主要功能：
* 支持用户登录/登出功能。
* 支持历史命令记录/查询。
* 支持backspace。
* 支持光标移动和编辑。
* 支持提示语设置。
* 内置help命令。

## 版本介绍

### release v1.0

- 配置参数：采用宏来配置 (适合把LwCLI作源码使用)
- 内存空间：预先分配
- 需指定最大的注册命令数

### release v2.0

- 配置参数：采用API来配置 (适合把LwCLI作为库使用)
- 内存空间：使用时才创建，节省空间

## 文件介绍

### cli_def.h

公共类型定义、公告API接口定义。

### list.h

双向链表。

### cli_line.c/h
实现提示语、退格、行解析、历史命令记录/查询。本模块无任何依赖，可单独作为库使用。

### cli_cmd.c/h
提供命令注册接口，支持help查询，复制解析命令行，并调用相关命令处理函数。本模块无任何依赖，可单独作为库使用。

### cli_shell.c/h
依赖cli_line和cli_command，创建line和command对象，并进一步接口封装，内置登录/登出功能，对外提供相关API接口。

## demo演示
编译和运行环境：ubuntu 16

### 编译
* cd /demo/linux
* make

### 运行
* ./build/lwcli_demo

### 登录
在终端界面输入用户名和密码

* Login: admin
* Password: admin

登录成功后，就可以输入命令，如：help等

## API接口介绍
* CliShellInit：初始化lwcli，参数需传入：依赖API、配置信息
* CliShellRegister：命令行注册函数
* CliShellInputChar：输入接口，输入单个字符给lwcli
* CliShellInputBlock：输入接口，输入块数据给lwcli
* CliShellTick：需要周期调用，用于命令行的解析和处理

## 初始化配置参数

### 依赖API

```c
typedef int32_t (*cli_printf_fn_t)(const char *format, ...);
typedef void   *(*cli_malloc_fn_t)(size_t size);
typedef void    (*cli_free_fn_t)  (void *base);

/* cli dependent interface */
typedef struct cli_api {
    cli_printf_fn_t printf_cb;    /* printf func pointer */
    cli_malloc_fn_t malloc_cb;    /* malloc func pointer */
    cli_free_fn_t   free_cb;      /* free func pointer */
} cli_api_t;
```

### 配置参数

```c
typedef struct cli_shell_cfg {
    int16_t line_queue_size;      // input data队列大小
    int16_t line_buf_size;        // 最大命令行长度
    int16_t line_history_num;     // 最大历史命令个数
    const char *username;         // NULL表示关闭用户登录功能
    const char *password;         // NULL表示关闭用户登录功能
    const char *prompt;           // 提示语
} cli_shell_cfg_t;
```


## 移植应用
* 拷贝lwcli文件夹到自己的工程源文件目录下
* IDE或者makefile文件中添加cli_line.c cli_cmd.c cli_shell.c三个源文件，添加lwcli头文件文件路径
* 调用CliShellInit进行初始化
* 调用CliShellInputChar/CliShellInputBlock来接收键盘等输入设备数据

p.s. 具体可以参考demo