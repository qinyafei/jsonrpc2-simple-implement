package main

import (
	"encoding/binary"
	"errors"
	"hash/adler32"
	"log"
)

//-------
//message format:
//length(4Byte) + "RPC2"(4Byte) + playload(N) + checksum(adler32,4Byte)
//-------

func PutI32BE(b []byte, v int32) {
	b[0] = byte(v >> 24)
	b[1] = byte(v >> 16)
	b[2] = byte(v >> 8)
	b[3] = byte(v)
}

func PutU32BE(b []byte, v uint32) {
	b[0] = byte(v >> 24)
	b[1] = byte(v >> 16)
	b[2] = byte(v >> 8)
	b[3] = byte(v)
}

func EncodePacket(payload []byte) []byte {
	var datalen int = len(payload)
	datalen += 4 + 4
	var pack []byte = make([]byte, datalen+4)
	PutI32BE(pack[:4], int32(datalen))
	copy(pack[4:8], "RPC2")
	copy(pack[8:], payload)

	var sum = adler32.Checksum(pack[:datalen])
	PutU32BE(pack[datalen:], sum)

	return pack
}

func DecodePacket(data []byte) (interface{}, error) {
	var totallen int = len(data)
	var dlen uint32 = binary.LittleEndian.Uint32(data[:4])
	if totallen < int(dlen)+4 {
		log.Println("packet data is not enough")
		return nil, errors.New("packet data is not enough")
	}

	if data[4] != 'R' || data[5] != 'P' || data[6] != 'C' || data[7] != '2' {
		log.Println("packet is confused")
		return nil, errors.New("packet is confused")
	}

	var compute uint32 = adler32.Checksum(data[:dlen])
	var realsum uint32 = binary.LittleEndian.Uint32(data[dlen:])
	if compute != realsum {
		log.Println("packet checksum failed")
		return nil, errors.New("packet checksum failed")
	}

	return data[8 : dlen-8], nil
}
