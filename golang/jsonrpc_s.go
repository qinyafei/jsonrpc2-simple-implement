package main

import (
	"log"
	"net"
	"os"
	"strconv"
)

type JsonRpcServer struct {
	ServiceMap map[string]*Service
}

type RpcConn struct {
}

type Service interface {
	Serve(req string) string
}

func NewRpcServer(port int) *JsonRpcServer {
	var serv *JsonRpcServer = new(JsonRpcServer)
	err := serv.StartRpcServer(port)
	if err != nil {
		log.Fatal("RpcServer started failed,", err.Error())
		os.Exit(1)
	}

	return serv
}

func (s *JsonRpcServer) StartRpcServer(port int) error {
	var addr string = "127.0.0.1:" + strconv.Itoa(port)
	servAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		log.Fatal("server addr is illegal")
		os.Exit(1)
	}

	listen, err2 := net.ListenTCP("tcp", servAddr)
	if err2 != nil {
		log.Fatal("listen ", addr, ",failed")
		os.Exit(1)
	}

	defer listen.Close()

	log.Println("server start successful")

	for {
		conn, err3 := listen.Accept()
		log.Println("accept tcp client,", conn.RemoteAddr().String())
		if err3 != nil {
			continue
		}

		go s.HandleConn(conn)
	}

}

func (s *JsonRpcServer) HandleConn(conn net.Conn) {
	//receive data

	//var method string
	//if serv, ok := s.ServiceMap[method]; !ok {
	//	log.Println("not found method:", method)
	//}

	//var response string = serv.Serve()
}

func (s *JsonRpcServer) RegisterService(method string, serv *Service) {
	s.ServiceMap[method] = serv
}
