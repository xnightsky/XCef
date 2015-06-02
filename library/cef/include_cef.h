#pragma once
// stl
#include "include_stl.h"

// cef
#include "lib_cef.h"

#include "include/cef_base.h"
#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"
#include "include/cef_client.h"
#include "include/cef_browser.h"
#include "include/cef_print_settings.h"
#include "include/cef_request.h"
#include "include/cef_v8.h"
#include "include/cef_stream.h"
#include "include/cef_task.h"
#include "include/cef_runnable.h"
#include "include/cef_url.h"
#include "include/cef_trace.h"
#if defined(CEF_USE_SANDBOX)
#include "include/cef_sandbox_win.h"
#endif

#include "include/base/cef_logging.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_bind_helpers.h"
#include "include/base/cef_callback.h"
#include "include/base/cef_callback_helpers.h"
#include "include/base/cef_callback_forward.h"
#include "include/base/cef_callback_list.h"
#include "include/base/cef_cancelable_callback.h"
#include "include/base/cef_weak_ptr.h"

#include "include/base/internal/cef_bind_internal.h"
#include "include/base/internal/cef_bind_internal_win.h"
#include "include/base/internal/cef_callback_internal.h"

#include "include/cef_stream.h"

#include "include/internal/cef_win.h"
#include "include/internal/cef_ptr.h"
#include "include/internal/cef_string.h"

#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_message_router.h"
#include "include/wrapper/cef_byte_read_handler.h"
#include "include/wrapper/cef_stream_resource_handler.h"
#include "include/wrapper/cef_helpers.h"

//#include "util.h"


// win
#include <windowsx.h>


