#ifndef _JSONRPCSERVER_H_
#define _JSONRPCSERVER_H

#include <string>
#include <stdlib.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#include "msgcodec.hpp"
#include "protocol.hpp"
#include <map>

#include "common.h"
#include "inspectservice.h"
#include "JsonWrap.h"

namespace jsonrpc2 {

#define MAX_LISTEN 1024
#define MAX_PACKET 1024 * 64


#define SERV_REPAIR "Rs.Repair.Submit"
#define SERV_DEVMAINTANANCE "Rs.DevMaintanance.Submit"
#define SERV_DEVREPAIR "Rs.DevRepair.Submit"
#define SERV_DEVCHECK "Rs.DevCheck.Submit"
#define SERV_INSPECT "Rs.Inspect.Submit"
#define SERV_SYSTEMSET "Rs.System.Set"
#define SERV_ALARMEVENT "Rs.AlarmEvent.Set"
#define SERV_DAYOFCAP "Rs.PicCapture.Set"

	class JsonRpcServer {

	public:
		JsonRpcServer() {}
		~JsonRpcServer() {}

		struct Conn {
			sockaddr cliAddr;
			int fd;
		};

	public:

		static Service* DynamicCreateService(std::string& method)
		{
			Service* serv = NULL;
			if (method == SERV_REPAIR)
			{
				serv = new Repair();
			}
			else if (method == SERV_DEVMAINTANANCE)
			{
				serv = new DevCare();
			}
			else if (method == SERV_DEVREPAIR)
			{
				serv = new DevRepair();
			}
			else if (method == SERV_DEVCHECK)
			{
				serv = new DevCheck();
			}
			else if (method == SERV_INSPECT)
			{
				serv = new SystemInspect();
			}
			else if (method == SERV_SYSTEMSET)
			{
				serv = new SystemSetting();
			}
			else if (method == SERV_ALARMEVENT)
			{
				serv = new AlarmEventRecord();
			}
			else if (method == SERV_DAYOFCAP)
			{
				serv = new PictureOfDay();
			}

			return serv;
		}


		int startServer(const char* ip, const int port) {
			int ret = 0;
			do {
				int servfd = socket(AF_INET, SOCK_STREAM, 0);
				if (servfd < 0) {
					ret = -1;
					break;
				}

				//SO_REUSEADDR
				int reuse = 1;
				setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

				sockaddr_in servaddr;
				memset(&servaddr, 0, sizeof(servaddr));
				servaddr.sin_family = AF_INET;
				servaddr.sin_addr.s_addr = inet_addr(ip);//htonl(INADDR_ANY);
				servaddr.sin_port = htons((unsigned short)port);
				ret = bind(servfd, (sockaddr*)&servaddr, sizeof(servaddr));
				if (ret < 0) {
					LOG(1, "server bind failed,errno=%d", errno);
					break;
				}

				ret = listen(servfd, MAX_LISTEN);
				if (ret < 0) {
					LOG(1, "server listen failed,errno=%d", errno);
					break;
				}

				while (true) {
					Conn* cli = new Conn();
					int socklen = sizeof(sockaddr);
					cli->fd = accept(servfd, (sockaddr*)&(cli->cliAddr), (socklen_t*)&socklen);
					if (cli->fd < 0) {
						LOG(1, "server accept failed,errno=%d", errno);
						break;
					}

					LOG(1, "new connection connected,fd=%d", cli->fd);
					pthread_t threadid;
					if (pthread_create(&threadid, NULL, &JsonRpcServer::threadProc, (void*)cli) != 0)
					{
						//error
					}
				}
			} while (false);

			return ret;
		}

		static void* threadProc(void* context) {
			int ret = 0;
			Conn* cnn = static_cast<Conn*>(context);
			if (cnn == NULL) {
				return NULL;
			}

			unsigned char* buf = NULL;
			const int maxpack = 1024 * 64;
			buf = new unsigned char[maxpack];
			if (buf == NULL) {
				return NULL;
			}

			do {
				memset(buf, 0, maxpack);
				int rlen = read(cnn->fd, buf, maxpack);
				if (rlen < 0) {
					LOG(1, "read failed,errno=%d", errno);
					break;
				}

				if (rlen == 0) {
					milliseconds_sleep(500);
					continue;
				}

				LOG(1, "fd:%d,receive data:%s", cnn->fd, buf + 4);

				//codec
				MsgCodec codec;
				unsigned char* payload = NULL;
				int actlen = 0;
				if (codec.decodeMsg(buf, rlen, &payload, actlen) != 0) {
					LOG(1, "decodeMsg failed,msg=%s", buf);
					continue;
				}

				//rpcdata
				RpcProtocol procol;
				Json::Value req;
				std::string strReq((char*)payload, actlen);
				if (procol.decodeRequest(strReq, req) != 0) {
					LOG(1, "decodeRequest failed,request=%s", payload);
					continue;
				}

				//deal request params and write response
				std::string response;
				std::string method = req["method"].asString();
				//TODO:什么时候释放？还是使用智能指针？
				Service* serv = JsonRpcServer::DynamicCreateService(method);
				if (serv == NULL)
				{
					LOG(1, "method not found,method=%s", method.c_str());
					continue;
				}

				std::string params;
				JsonWrap json;
				json.JsonToString(req["params"], params);
				ret = (*serv)(params, response);
				if (ret != 0)
				{
					LOG(1, "bad request,method=%s", method.c_str());
					continue;
				}

				//make response
				memset(buf, 0, maxpack);
				codec.encdoeMsg((const unsigned char*)response.c_str(), response.length(), buf, maxpack, actlen);
				int wlen = write(cnn->fd, buf, actlen);
				if (wlen != actlen) {
					LOG(1, "write response failed,response=%s", response.c_str());
					break;
				}

			} while (true);

			delete[]buf;
			buf = NULL;

			LOG(1, "client connection thread exit...");

			return NULL;
		}

	};

}




#endif
