#pragma once
#include "default.h"
#include "object.h"

#define SIMPLEJS_PLUGIN_ENTRY_NAME "plugin_main"
typedef simplejs_status_t (*simplejs_plugin_entry_f)(simplejs_object_t *global_object);
