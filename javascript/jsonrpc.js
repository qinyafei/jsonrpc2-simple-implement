//JsonRpc2.0 javascript implementation
//---
//version:1.0.0
//---

/*
define variable type:
str
obj
arr
json
*/

var Request = function (m, param, cid) {
    this.method = m;
    this.params = param;
    this.id = cid;
};

var Response = function () {
    this.result = undefined;
    this.id = -1;
    this.error = undefined;
};


var JsonRpc2 = {
    CallId: 1,
    GetCallId: function () {
        var callid = JsonRpc2.CallId++;
        return callid;
    },
    CreatRpc: function () {
        var jsonrpc = {};
        jsonrpc.version = "2.0";

        jsonrpc.rpcall = function (url_str, req_obj) {
            var strReq = jsonrpc.makeRpcBody(req_obj);
            return new Promise(function (resolve, reject) {
                var xhr = new XMLHttpRequest();
                //xhr.responseType = "text";
                xhr.open('POST', url_str, true);
				xhr.setRequestHeader('X-Request-With', null);
                xhr.timeout = 10000;
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        //var r = "response:" + xhr.response;
                        //alert(unescape(r));
                        var res = JSON.parse(xhr.response);
                        var res_obj = jsonrpc.parseResponse(res);

                        resolve(res_obj);
                    }
                };
                xhr.ontimeout = function (e) {
                    var res = new Response();
                    res.error = JSON.parse("{\"error\":\"rpc request timeout\"}");
                    res.id = -1;
                    reject(res);
                };
                xhr.onerror = function (e) {
                    var res = new Response();
                    res.error = JSON.parse("{\"error\":\"rpc request occured unknow error\"}");
                    res.id = -2;
                    reject(res);
                };
                xhr.upload.onprogress = function (e) { };

                //
                xhr.send(strReq);
            });
        };

        jsonrpc.rpcallbatch = function (callarray_objarr) {
            var strReq = "[";
            for (var i = 0; i < callarray_objarr.length; i++) {
                var sone = jsonrpc.makeRpcBody(callarray_objarr[i]);
                strReq += sone;
                if (i + 1 < callarray_objarr.length) {
                    strReq += ",";
                }
            }
            strReq += "]";

            return new Promise(function (resolve, reject) {
                var xhr = new XMLHttpRequest();
                //xhr.responseType = "text";
                xhr.open('POST', url, true);
                xhr.timeout = 10000;
                xhr.onreadystatechange = function () {
                    if (xhr.readyState == 4 && xhr.status == 200) {
                        //var r = "response:" + xhr.response;
                        //alert(unescape(r));
                        var res_arrobj = new Array();
                        var res = JSON.parse(xhr.response);
                        for (var i = 0; i < res.length; i++) {
                            var res_obj = jsonrpc.parseResponse(res);
                            res_arrobj.push(res_obj);
                        }

                        resolve(res_arrobj);
                    }
                };
                xhr.ontimeout = function (e) {
                    var res = new Response();
                    res.error = JSON.parse("{\"error\":\"rpc request timeout\"}");
                    res.id = -1;
                    reject(res);
                };
                xhr.onerror = function (e) {
                    var res = new Response();
                    res.error = JSON.parse("{\"error\":\"rpc request occured unknow error\"}");
                    res.id = -2;
                    reject(res);
                };
                xhr.upload.onprogress = function (e) { };

                //
                xhr.send(strReq);
            });
        };

        jsonrpc.makeRpcBody = function (req_obj) {
            var body = "{\"jsonrpc\":\"" + jsonrpc.version + "\",";
            body += "\"method\":\"" + req_obj.method + "\",";
            body += "\"params\":" + req_obj.params;
            if (req_obj.id != undefined) {
                body += ",";
                body += "\"id\":" + String(req_obj.id);
            }
            body += "}"

            return body;
        };

        jsonrpc.parseResponse = function (res_jsonobj) {
            var res = new Response();
            res.id = res_jsonobj["id"];
            if ("result" in res_jsonobj == true) {
                res.result = res_jsonobj["result"];
            } else if ("error" in res_jsonobj == true) {
                res.error = res_jsonobj["error"];
            }
            return res;
        };

        return jsonrpc;
    }
}




