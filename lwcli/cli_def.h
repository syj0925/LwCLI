/**
 * @brief cli public definition.
 * @file cli_def.h
 * @version 1.0
 * @author Su YouJiang
 * @date 2020-3-17 19:5:59
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-17 19:5:59
 *   Author      : Su YouJiang
 *   Modification: Created file
 */

#ifndef __CLI_DEF_H__
#define __CLI_DEF_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

enum {
    RET_OK = 0,
    RET_ERROR = -1,
};

typedef int32_t (*cli_printf_fn_t)(const char *format, ...);
typedef void   *(*cli_malloc_fn_t)(size_t size);
typedef void    (*cli_free_fn_t)  (void *base);

/* cli interface */
typedef struct cli_api {
    cli_printf_fn_t printf_cb;    /* printf func pointer */
    cli_malloc_fn_t malloc_cb;    /* malloc func pointer */
    cli_free_fn_t   free_cb;      /* free func pointer */
} cli_api_t;


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_DEF_H__

