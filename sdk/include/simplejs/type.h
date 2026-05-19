#pragma once

// Workaround for MSVC declaration parsing issue when using SIMPLEJS_API with pointer return types

#define SIMPLEJS_TYPEDEF(type_t, type) \
    typedef const type_t const_##type; \
    typedef type_t type

SIMPLEJS_TYPEDEF(void *, pvoid);
SIMPLEJS_TYPEDEF(char *, pchar);
