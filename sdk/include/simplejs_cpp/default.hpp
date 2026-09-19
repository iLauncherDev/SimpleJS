#pragma once
#include <simplejs/default.h>
#include <type_traits>

#define simplejs_striptype_strict(type) std::remove_cv_t<type>
#define simplejs_striptype_soft(type) std::remove_cvref_t<type>

#define simplejs_decltype_strict(value) simplejs_striptype_strict(decltype(value))
#define simplejs_decltype_soft(value) simplejs_striptype_soft(decltype(value))
