# XCef
Chromiumembedded[CEF] demo

这是 CEF的一部分例子...

配置环境：  
1.   cef配置  
    1.1 CEF版本是[cef_binary_3.2171.1979_windows64](http://www.magpcss.net/cef_downloads/index.php?file=cef_binary_3.2171.1979_windows64.7z)
    1.2 导入"\library\cef\include_cef.h"或者直接使用"src\include_XCef.h"包含全部xcef+cef内容
    1.3 有时需要修改CEF_LIB_DIR宏来重新定位cef lib位置  
    1.4 cef dll和相关资源文件，请根据1.1中的链接自行下载和编译  
2.  需要VS2013以上版本IDE编译  
3.  如果需要编译wtl项目，wtl版本WTL90_4140_Final  
4.  cJson已经包含，导入"\library\cJSON\include_cJSON.h"  

include eg:  
```cpp  
    #include "..\\library\\cJSON\include_cJSON.h"  
    #define CEF_LIB_DIR "..\\library\\cef\\lib64\\"  
    #include "..\\library\\cef\\include_cef.h"  
    #include "..\src\include_XCef.h"  
```  

:)
