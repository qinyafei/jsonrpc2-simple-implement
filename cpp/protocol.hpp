#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "JsonWrap.h"

namespace jsonrpc2 {


struct Request {
    std::string version;
    std::string method;
    Json::Value params;
    int id;
};


struct Response {
	std::string version;
	std::string result;
	int id;
};

enum ErrorCodes {
	eParseError = -32700,
	eInvalidRequest = -32600,
	eMethodNotFound  = -32601,
	eInvalidParams = -32602,
	eInternalError = -32603,
	eServerErrorStart = -32099,
	eServerErrorEnd = -32000
};


class RpcProtocol {

public:
	static int decodeRequest(std::string& req, Json::Value& jsonReq) {
		int ret = 0;

		do {
			JsonWrap json;
			ret = json.ParseString(req.c_str());
			if (ret != 0)
			{
				break;
			}
			jsonReq = json.m_JsonRoot;

		} while (false);

			return ret;
	}

	//{"result":""}
	//{"error":""}
	//{"jsonrpc": "2.0", "error": {"code": -32700, "message": "Parse error"}, "id": null}
	//{"jsonrpc": "2.0", "result" : -19, "id" : 2}
	static int encodeResponse(Json::Value& val, int id, std::string& outRes)
	{
		static const std::string version = "2.0";
		std::string res;
		JsonWrap json;
		json.JsonToString(val, res);

		outRes = "{\"jsonrpc\":";
		outRes += version + ",";
		outRes += res + ",";
		outRes += "\"id\":";
		char tmp[32] = { 0 };
		sprintf(tmp, "%d", id);
		outRes += tmp;
		outRes += "}";
	}

};


    
}


#endif