#ifndef _MSGCODEC_H_
#define _MSGCODEC_H_

#include <string>
#include <string.h>

#include "common.h"


#include <zlib.h>

namespace jsonrpc2 {


int checksum(const void* buf, int len)
{
	return static_cast<int>(
		adler32(1, static_cast<const char*>(buf), len));
}


//msg packet
//len(4Byte) + "RPC2"(4Byte) + payload(N) + checksum(4Byte)
class MsgCodec {
	
public:
	static int decodeMsg(unsigned char* msg, int msglen, unsigned char** payload, int& actlen) {
		unsigned char* tmp = msg;

		uint32_t len = 0;
		uint32_t bit4 = 0;
		bit4 = tmp[0];
		len |= bit4 << 24;
		bit4 = tmp[1];
		len |= bit4 << 16;
		bit4 = tmp[2];
		len |= bit4 << 8;
		bit4 = tmp[3];
		len |= bit4;

		if (len >= msglen) {
			return -1;
		}

		unsigned char mark[4] = { 0 };
		mark[0] = tmp[4];
		mark[1] = tmp[5];
		mark[2] = tmp[6];
		mark[3] = tmp[7];
		if (mark[0] != 'R' || mark[1] != 'P' || mark[2] != 'C' || mark[3] != '2') {
			return -2;
		}

		//payload length
		actlen = len - 8;
		//check sum
		uint32_t chksum = 0;
		bit4 = tmp[len];
		chksum |= bit4 << 24;
		bit4 = tmp[len + 1];
		chksum |= bit4 << 16;
		bit4 = tmp[len + 2];
		chksum |= bit4 << 8;
		bit4 = tmp[len + 3];
		chksum |= bit4;

		int comput = checksum(tmp, len);
		if (chksum != comput) {
			return -3;
		}

		*payload = tmp + 8;
		LOG(1, "receive json request:%s", payload);

		return 0;
	
	}

	static int encdoeMsg(const unsigned char* payload, int datalen, unsigned char* outmsg, int memlen, int& actlen) {
		int len = datalen + 4 + 4;
		if (memlen <= len) {
			return -1;
		}

		unsigned char* tmp = outmsg;

		tmp[0] = (unsigned char)(len & 0xFF000000);
		tmp[1] = (unsigned char)(len & 0x00FF0000);
		tmp[2] = (unsigned char)(len & 0x0000FF00);
		tmp[3] = (unsigned char)(len & 0x000000FF);
		tmp += 4;
		//RPC2
		unsigned char* mark = "RPC2";
		memcpy(tmp, mark, 4);
		tmp += 4;
		memcpy(tmp, payload, datalen);
		tmp += datalen;


		int chksum = checksum((void*)outmsg, len);
		tmp[0] = (unsigned char)(chksum & 0xFF000000);
		tmp[1] = (unsigned char)(chksum & 0x00FF0000);
		tmp[2] = (unsigned char)(chksum & 0x0000FF00);
		tmp[3] = (unsigned char)(chksum & 0x000000FF);

		actlen = len + 4;
		return 0;
	}
};


}

#endif