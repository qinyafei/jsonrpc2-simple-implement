package main

import (
	"encoding/json"
	"log"
	"strconv"
)

//definition
type Request struct {
	version string `json:"jsonrpc"`
	method  string `json:"method"`
	params  string `json:"params"`
	id      int    `json:"id"`
}

type Response struct {
	version string `json:"jsonrpc"`
	result  string `json:"result"`
	id      int    `json:"id"`
}

type ResponseError struct {
	version string `json:"jsonrpc"`
	error   string `json:"error"`
	id      int    `json:"id"`
}

func EncodeRequest2(req Request) string {
	var body []byte
	var err error
	body, err = json.Marshal(req)
	if err != nil {
		log.Println("EncodeRequest2 json.Marshal failed")
	}

	return string(body)
}

func EncodeRequest(req Request) string {
	var rpcversion string = "2.0"
	var body string = "{\"jsonrpc\":\"" + rpcversion + "\""
	body += ",\"method\":\"" + req.method + "\""
	body += ",\"params\":" + req.params
	body += ",\"id\":" + strconv.Itoa(req.id)
	body += "}"

	return body
}

func EncodeRequestBatch2(reqArr []Request) string {
	var body []byte
	var err error
	body, err = json.Marshal(reqArr)
	if err != nil {
		log.Println("json.Marshal failed")
	}

	return string(body)
}

func EncodeRequestBatch(reqArr []Request) string {
	var body string
	body = "["
	alen := len(reqArr)
	for index, val := range reqArr {
		var req = EncodeRequest(val)
		body += req
		if index+1 < alen {
			body += ","
		}
	}

	body += "]"

	return body
}

//func JsonDictionary(jsons interface{}) (interface{}, error) {
//
//}

func toString(s interface{}) string {
	var res string
	switch v := s.(type) {
	case string:
		res = s.(string)
		log.Println(v)
		break
	default:
		return ""
		break
	}

	return res
}

func toInt(s interface{}) int {
	var in int
	switch v := s.(type) {
	case int:
	case uint32:
	case uint16:
	case int16:
	case int32:
		in = s.(int)
		log.Println(v)
		break
	default:
		return -1
		break
	}

	return in
}

func DecodeRequest2(req string) Request {
	var struReq Request
	var err error
	var jsonmap map[string]interface{}
	if err = json.Unmarshal([]byte(req), &jsonmap); err != nil {
		log.Println("json.Unmarshal failed,", err.Error())
	}

	//struReq.version = toString(jsonmap["jsonrpc"])
	//struReq.method = toString(jsonmap["method"])
	//struReq.id = toInt(jsonmap["id"])
	var ok bool = false
	struReq.version, ok = jsonmap["jsonrpc"].(string)
	if !ok {
		//
	}

	struReq.method, ok = jsonmap["method"].(string)
	if !ok {
		//
	}

	struReq.id, ok = jsonmap["id"].(int)
	if !ok {
		//
	}

	var pa []byte
	if pa, err = json.Marshal(jsonmap["params"]); err != nil {
		log.Println("json.Marshal failed")
	}
	struReq.params = string(pa)

	return struReq
}

func DecodeRequestBatch(req string) []Request {
	var struReq []Request
	if err := json.Unmarshal([]byte(req), &struReq); err != nil {
		log.Println("json.Unmarshal failed")
	}

	return struReq
}

func EncodeResponse(res Response) string {
	var rpcversion string = "2.0"
	var body string = "{\"jsonrpc\":\"" + rpcversion + "\""
	body += ",\"result\":" + res.result
	body += ",\"id\":" + strconv.Itoa(res.id)
	body += "}"

	return body

}

func EncodeErrorResponse(res ResponseError) string {
	var strRes []byte
	var err error
	if strRes, err = json.Marshal(res); err != nil {
		log.Println("json marshal failed")
	}

	return string(strRes)
}
