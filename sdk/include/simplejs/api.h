#pragma once

#if defined(_WIN32)
#define SIMPLEJS_DLL_EXPORT __declspec(dllexport)
#define SIMPLEJS_DLL_IMPORT __declspec(dllimport)
#define SIMPLEJS_API_CALL __fastcall
#elif defined(__GNUC__) || defined(__clang__)
#define SIMPLEJS_DLL_EXPORT __attribute__((visibility("default")))
#define SIMPLEJS_DLL_IMPORT __attribute__((visibility("default")))
#define SIMPLEJS_API_CALL
#else
#error "Unknown compiler!"
#endif

#define SIMPLEJS_PLUGIN_API SIMPLEJS_DLL_EXPORT SIMPLEJS_API_CALL

#ifdef SIMPLEJS_HOOK
#define SIMPLEJS_HOOK_API SIMPLEJS_DLL_EXPORT SIMPLEJS_API_CALL
#else
#define SIMPLEJS_HOOK_API SIMPLEJS_DLL_IMPORT SIMPLEJS_API_CALL
#endif

#ifdef SIMPLEJS_CORE
#define SIMPLEJS_API SIMPLEJS_DLL_EXPORT SIMPLEJS_API_CALL
#else
#define SIMPLEJS_API SIMPLEJS_DLL_IMPORT SIMPLEJS_API_CALL
#endif
