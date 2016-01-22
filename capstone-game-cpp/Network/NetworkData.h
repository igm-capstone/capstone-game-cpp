#pragma once
#include <string.h>

#define DEFAULT_PORT "7895" 
#define MAX_DATA_SIZE 1000000
#define MAX_CLIENTS 4

enum PacketTypes {
	INIT_CONNECTION = '0',
	GIVE_ID = '1',
	SPAWN_EXPLORER = 'E',
	GRANT_AUTHORITY = 'A',
	SYNC_TRANSFORM = 'T',
	UNKNOWN
};

struct Packet {
	unsigned char Type;
	
	union {
		unsigned int UUID;
	}; //Target GO 

	union {
		unsigned int ClientID;
	}; // Data
	
	union {
		float Position[4];
	}; // Float Data

	Packet() : Type(PacketTypes::UNKNOWN) {};
	Packet(PacketTypes t) : Type(t) {};

	void Serialize(char* data) {
		memcpy(data, this, sizeof(Packet));
	}

	void Deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};