(
    function(w) {
        /*
            src: http://stackoverflow.com/questions/10527983/best-way-to-detect-mac-os-x-or-windows-computers-with-javascript-or-jquery
            Mac Computers
            Mac68K Macintosh 68K system.
            MacPPC Macintosh PowerPC system.
            MacIntel Macintosh Intel system.
            iOS Devices

            iPhone iPhone.
            iPod iPod Touch.
            iPad iPad.
        */
        function isMacintosh() {
            return navigator.platform.indexOf('Mac') > -1
        }

        function isWindows() {
            return navigator.platform.indexOf('Win') > -1
        }
        function isIOS(){
            var u = navigator.userAgent, app = navigator.appVersion;
            var isiOS = !!u.match(/\(i[^;]+;( U;)? CPU.+Mac OS X/);
            console.log("ios");
            return isiOS;
        }
        //
        function getjsonrpc(method, params, persistent, rpc_id, protocol){
            //{"jsonrpc": "2.0", "protocol": "app://", "method": "sum", "params": [1,2,4], "id": "0"}
            if (!method)
                return null;
            if ("undefined" == typeof(params)) {
                params = null;
            } else if ("array" != typeof(params)) {
                var param_array = [];
                param_array.push(params);
                params = param_array;
            }
            if (!rpc_id)
                rpc_id = 0;
            if (!protocol)
                protocol = "app://";

            // 实现部分 jsonrpc 2.0 功能
            return {
                "jsonrpc": "2.0",
                "protocol": protocol,
                "method": method,
                "params": params,
                "id": rpc_id
            };
        }
        //
        function guid() {
            function s4() {
                return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
            }
            return "guid_" + s4() + s4() + '_' + s4() + '_' + s4() + '_' +
            s4() + '_' + s4() + s4() + s4();
        }
        function create_queryinfo(data)
        {
            return {
                push: function (key, reqmsg, cb_success, cb_failure) {
                    q = {
                        'key': key,
                        'reqmsg': reqmsg,
                        'on_success': cb_success,
                        'on_failure': cb_failure
                    }
                    data[key] = q;
                    return q;
                },
                pop: function (q, force) {
                    var force = arguments[1] ? arguments[1] : false;
                    if (force || !q.reqmsg.persistent) {
                        delete data[q.key];
                    }
                },
                success: function (key, response) {
                    if (data.hasOwnProperty(key)) {
                        q = data[key];
                        q.on_success(response);
                        this.pop(q);
                    } else {
                        alert("Not find key!");
                    }
                },
                failure: function (key, error_code, error_message) {
                    if (data.hasOwnProperty(key)) {
                        q = data[key];
                        q.on_failure(error_code, error_message);
                        this.pop(q, true);
                    }
                }
            };
        }

        if(isWindows() || isMacintosh()){
            if (!w || !w.Application) {
                // test api or error， only windows or mac support it
                console.error("Not find window.Application!");
                return
            }
        }

        if (isWindows()) {
            if ("undefined" == typeof(w.Application.cefQuery)) {
                console.error("window.cefQuery is not load");
            }

            function jsonrpcQuery(method, params, cb_success, cb_failure, persistent, rpc_id, protocol) {
                var jsonrpc = getjsonrpc(method, params, persistent, rpc_id, protocol);
                if (!jsonrpc)
                    return 0;
                if ("undefined" == typeof(persistent))
                    persistent = false;
                var str_jsonrpc = JSON.stringify(jsonrpc);
                //alert(str_jsonrpc); return;
                var request_message = {
                    request: str_jsonrpc,
                    persistent: persistent,
                    onSuccess: function(response) {},
                    onFailure: function(error_code, error_message) {}

                }
                if (cb_success)
                    request_message.onSuccess = cb_success;
                else
                    request_message.onSuccess = function(response) {};
                if (cb_failure)
                    request_message.onFailure = cb_failure;
                else
                    request_message.onFailure = function(error_code, error_message) {
                        console.error("[" + error_code + "]:" + error_message)
                    };

                var request_id = w.cefQuery(request_message);
                return request_id;
            }

            w.jsonrpcQuery = jsonrpcQuery;
        } else if (isMacintosh()) {
            if ("undefined" == typeof(w.Application.cefQueryOSX)) {
                console.error("window.cefQueryOSX is not load");
            }

            var query_info = {};
            var query_mng = create_queryinfo(query_info);

            function jsonrpcQueryMac(method, params, cb_success, cb_failure, persistent, rpc_id, protocol) {
                var jsonrpc = getjsonrpc(method, params, persistent, rpc_id, protocol);
                if (!jsonrpc)
                    return 0;
                if ("undefined" == typeof(persistent))
                    persistent = false;
                var str_jsonrpc = JSON.stringify(jsonrpc);
                //alert(str_jsonrpc); return;
                var key = guid();

                var request_message = {
                    request: str_jsonrpc,
                    persistent: persistent,
                    cb_key: key
                };
                if (!cb_success)
                    cb_success = function(response) {};
                if (!cb_failure){
                    cb_failure = function(error_code, error_message) {
                        console.error("[" + error_code + "]:" + error_message)
                    };
                }

                var q = query_mng.push(key, request_message, cb_success, cb_failure);

                w.cefQueryOSX(request_message);
                return key;
            }


            w.jsonrpcQuery = jsonrpcQueryMac;
            w.jsonrpcQuery.success = query_mng.success;
            w.jsonrpcQuery.failure = query_mng.failure;
            w.cefQuery = function(){
                return w.Application.cefQuery.apply(w.Application, arguments);
            };
            if(w.Application.hasOwnProperty("cefQueryCancelOSX")){
                w.cefQueryCancel = function(){
                    return w.Application.cefQueryCancelOSX.apply(w.Application, arguments);
                };
            }

            // OS call js nativate function:
            // window.Application.cefQueryOSX
            //
            // OS calls js callback:
            // window.jsonrpcQuery.success
            // window.jsonrpcQuery.failure
        } else if(isIOS()){
            var query_info = {};
            var query_mng = create_queryinfo(query_info);

            function jsonrpcQueryIOS(method, params, cb_success, cb_failure, persistent, rpc_id, protocol) {
                var jsonrpc = getjsonrpc(method, params, persistent, rpc_id, protocol);
                if (!jsonrpc)
                    return 0;
                if ("undefined" == typeof(persistent))
                    persistent = false;

                var str_jsonrpc = JSON.stringify(jsonrpc);
                //alert(str_jsonrpc); return;
                var key = guid();

                var request_message = {
                    request: str_jsonrpc,
                    persistent: persistent,
                    cb_key: key
                };
                if (!cb_success)
                    cb_success = function(response) {};
                if (!cb_failure){
                    cb_failure = function(error_code, error_message) {
                        console.error("[" + error_code + "]:" + error_message)
                    };
                }

                var q = query_mng.push(key, request_message, cb_success, cb_failure);

                //////// turn to the message ios can received/////////
                var request_message_string =JSON.stringify(request_message);
                w.cefQuery(request_message_string);
                /////////////////
                return key;
            }


            w.jsonrpcQuery = jsonrpcQueryIOS;
            w.jsonrpcQuery.success = query_mng.success;
            w.jsonrpcQuery.failure = query_mng.failure;
            w.cefQuery = function(jsonstr){
                w.location.href = "http://cefQueryIOS/" + jsonstr;
            }
            w.cefQueryCancel = function(reqid){
                w.location.href = "http://cefQueryCancelIOS/" + reqid;
            }

            log("ios");
        }
        else{
            console.error("Unknown Platform!");
        }
    }
)(window);