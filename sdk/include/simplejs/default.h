#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "api.h"
#include "type.h"
#include "hook.h"
#include "lib/spinlock.h"
#include "lib/status.h"
#include "lib/list.h"
#include "lib/safe_list.h"
#include "lib/s_string.h"

#define FUNC_IS_ON_STRING(name, vector)                               \
    bool name(char *string)                                           \
    {                                                                 \
        for (size_t i = 0; i < sizeof(vector) / sizeof(*vector); i++) \
        {                                                             \
            if (!strcmp(vector[i], string))                           \
                return true;                                          \
        }                                                             \
        return false;                                                 \
    }

#define SIMPLEJS_ASSERT(cond)                                      \
    if (!(cond))                                                   \
    {                                                              \
        fprintf(stderr, "ASSERT FAILED: %s\nFile: %s\nLine: %d\n", \
                #cond, __FILE__, __LINE__);                        \
        abort();                                                   \
    }

#define SIMPLEJS_REQUIRE(condition, label) \
    if (!condition)                        \
    {                                      \
        goto label;                        \
    }

#define string_includes(str, chr) ((strchr(str, chr)) != NULL)
#define memclr(p, s) memset(p, 0, s)

#define SIMPLEJS_CLASS_CONSTRUCTOR_PROPERTY "constructor"
#define SIMPLEJS_CLASS_PROTOTYPE_PROPERTY "prototype"

#define CASE_TO_STRING(name) \
    case name:               \
        return #name

simplejs_status_t SIMPLEJS_API simplejs_init();
void SIMPLEJS_API simplejs_uninit();
