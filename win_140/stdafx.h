#define SECURITY_WIN32
#pragma once

#ifndef __cplusplus
#	error requires C++
#endif

#define DECLSPEC_DEPRECATED_DDK

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#define _NO_CRT_STDIO_INLINE
#define _NO_CPP_INLINES
#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 0
#define _ALLOW_COMPILER_AND_STL_VERSION_MISMATCH
#define __EDG__
#define USE_ATL_THUNK2

#ifndef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT __declspec(dllimport)
#endif

#define DPAPI_IMP DECLSPEC_IMPORT
#define _CRTIMP DECLSPEC_IMPORT
#define _CRTIMP_ALT DECLSPEC_IMPORT

#define _NT_BEGIN namespace NT {
#define _NT_END }

#pragma warning(disable : 4073 4074 4075 4097 4514 4005 4200 4201 4238 4307 4324 4480 4530 4706 5040)

#include <stdlib.h>
//#include <wchar.h>
#include <stdio.h>
#include <string.h>

#define _INC_MMSYSTEM  /* Prevent inclusion of mmsystem.h in windows.h */

#include <Windows.h>
#include <intrin.h>

#undef _INC_MMSYSTEM

#include <functional>
#include <ppltasks.h>
