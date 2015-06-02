#pragma once

// lib_cef 的文件位置自己调
#define CEF_LIB_DIR "..\\Cef\\Lib\\"


#ifdef _DEBUG
#pragma comment(lib, CEF_LIB_DIR "libcef_dll_wrapperD.lib")
#else
#pragma comment(lib, CEF_LIB_DIR "libcef_dll_wrapper.lib")
#endif

#pragma comment(lib, CEF_LIB_DIR "libcef.lib")
