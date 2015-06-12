function jsonrpcQuery(method, params, cb_success, cb_failure, persistent, rpc_id, protocol){
	//{"jsonrpc": "2.0", "protocol": "app://", "method": "sum", "params": [1,2,4], "id": "0"}
	if(!method)
		return 0;
	if("undefined" == typeof(params))
	{	
		params = null;
	}
	else if("array" != typeof(params))
	{
		var param_array = [];
		param_array.push(params);
		params = param_array;
	}
	if(!rpc_id)
		rpc_id = 0;
	if(!protocol)
		protocol = "app://";
	if("undefined" == typeof(persistent))
		persistent = false;

	// 实现部分 jsonrpc 2.0 功能
	var jsonrpc = {
		"jsonrpc": 	"2.0", 
		"protocol": protocol,
		"method": 	method, 
		"params": 	params, 
		"id": 		rpc_id
		};
	var str_jsonrpc = JSON.stringify(jsonrpc);
	//alert(str_jsonrpc);	return;
	var request_message = {
		request: str_jsonrpc,
		persistent: false,
		//onSuccess: function(response) {},
		//onFailure: function(error_code, error_message) {}
	}
	if(cb_success)
		request_message.onSuccess = cb_success;
	else
		request_message.onSuccess = function(response) {};
	if(cb_failure)
		request_message.onFailure = cb_failure;
	else
		request_message.onFailure = function(error_code, error_message) { console.log("[" + error_code + "]:"+ error_message)};

	var request_id = window.cefQuery(request_message);
	return request_id;
}