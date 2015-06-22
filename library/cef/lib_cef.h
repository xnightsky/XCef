#pragma once

// lib_cef 的文件位置自己调
#ifndef CEF_LIB_DIR
#define CEF_LIB_DIR "\\lib64\\"
#endif

#ifdef _DEBUG
#define CEF_LIB_SUBDIR "Debug\\"
#else
#define CEF_LIB_SUBDIR "Release\\"
#endif

#pragma comment(lib, CEF_LIB_DIR CEF_LIB_SUBDIR "libcef_dll_wrapper.lib")
#pragma comment(lib, CEF_LIB_DIR CEF_LIB_SUBDIR "libcef.lib")
#if defined(CEF_USE_SANDBOX)
// The cef_sandbox.lib static library is currently built with VS2013. It may not
// link successfully with other VS versions.
#pragma comment(lib, CEF_LIB_DIR CEF_LIB_SUBDIR "cef_sandbox.lib")
#endif


