package main

import (
	"errors"
	"log"
	"net"
	"os"
	"strconv"
	"time"
)

type JsonRpc2Client struct {
	ip      string
	port    int
	netConn *TcpClient
}

type ResultFunc func(results string)

type TcpClient struct {
	conn     *net.TCPConn
	servAddr *net.TCPAddr
}

func NewTcpConn(ip string, port int) *TcpClient {
	var conn *TcpClient = new(TcpClient)
	var addr = ip + ":" + strconv.Itoa(port)
	servAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		log.Fatal("remote server addr is illegal,error:", err.Error())
		os.Exit(-1)
	}

	tcpconn, err := net.DialTCP("tcp", nil, servAddr)
	if err != nil {
		log.Fatal("remote server connect failed,", err.Error())
		os.Exit(-2)
	}

	conn.servAddr = servAddr
	conn.conn = tcpconn

	return conn
}

//eg:
func NewRpcClient(servip string, servport int) *JsonRpc2Client {
	rpc := &JsonRpc2Client{
		ip:   servip,
		port: servport,
	}
	rpc.netConn = NewTcpConn(servip, servport)

	return rpc
}

//implement JsonRpc2Client//
func (rpc *JsonRpc2Client) SyncRpc(req Request, resCall ResultFunc) {
	var body = EncodeRequest2(req)
	var resultChan chan string = make(chan string)
	go rpc.SendRequest(body, resultChan, 0)

	for {
		select {
		case res := <-resultChan:
			resCall(res)
			return
		case <-time.After(5 * time.Second):
			log.Println("rpc read response timeout")
			return
		}
	}

}

func (rpc *JsonRpc2Client) NoResponseRpc(req Request) {
	var body string = EncodeRequest(req)
	var resultChan chan string = make(chan string)
	rpc.SendRequest(body, resultChan, 1)
}

func (rpc *JsonRpc2Client) AsyncRpc(req Request, resChan chan string) {
	var body = EncodeRequest2(req)
	go rpc.SendRequest(body, resChan, 0)
}

func (rpc *JsonRpc2Client) SendRequest(req string, res chan string, noresp int) error {
	log.Print("###SendRequest:", req)
	for {
		var encReq []byte = EncodePacket([]byte(req))
		wlen, err := rpc.netConn.conn.Write(encReq)
		if len(encReq) != wlen || err != nil {
			log.Print("SendRequest failed,wlen=", wlen, err.Error())
			return errors.New("send request failed")
		}
		//deal send uncomplete
		break
	}

	//no response
	if noresp == 1 {
		return nil
	}

	//receive response
	var maxresponsesize int = 1024 * 64
	var response []byte = make([]byte, maxresponsesize)
	for {
		rlen, err := rpc.netConn.conn.Read(response)
		if rlen <= 0 || err != nil {
			log.Println("read response failed")
			return errors.New("read response failed")
		}

		pack, err := DecodePacket(response)
		if err != nil {
			log.Println("decode packet error")
			//TODO:
			return errors.New("decode packet error")
		}

		res <- pack.(string)
		break
	}

	return nil
}
