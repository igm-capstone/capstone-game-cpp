#pragma once
#include <string.h>

#define DEFAULT_PORT "7895" 
#define MAX_PACKET_SIZE 1000000
#define MAX_CLIENTS 4

enum PacketTypes {
	INIT_CONNECTION = 0,
	CHNAGE_COLOR = 'c',
};

struct Packet {
	unsigned char Type;
	unsigned int Argument;

	void Serialize(char* data) {
		memcpy(data, this, sizeof(Packet));
	}

	void Deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};