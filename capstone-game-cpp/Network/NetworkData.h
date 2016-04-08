#pragma once
#include <string.h>

#define DEFAULT_PORT "7895" 
#define MAX_DATA_SIZE 1000000

enum PacketTypes {
	INIT_CONNECTION = '0',
	SET_CLIENT_ID	= '1',
	SPAWN_EXPLORER	= 'E',
	SPAWN_MINION	= 'M',
	GRANT_AUTHORITY = 'A',
	SYNC_TRANSFORM	= 'T',
	SYNC_HEALTH		= 'H',
	SYNC_ANIMATION	= 'N',
	SPAWN_HEAL		= 'h',
	UNKNOWN
};

struct Packet {
	unsigned char Type = UNKNOWN;
	
	union {
		unsigned int UUID = 0;
	}; //Target GO 

	union {
		unsigned int ClientID = 0;
		unsigned int MinionType;
	}; // Data

	union
	{
		struct 
		{
			vec3f Position = { 0, 0, 0 };
			quatf Rotation = { 0, 0, 0, 1 };
		} AsTransform;

		struct
		{
			byte State;
			byte Command;
		} AsAnimation;

		struct
		{
			vec3f Position = { 0, 0, 0 };
			float Duration = 0;
		} AsSkill;
		
		float AsFloat;
	}; // Package Specific Data

	Packet() : Type(UNKNOWN) {};
	Packet(PacketTypes t) : Type(t) {};

	void Serialize(char* data) const {
		memcpy(data, this, sizeof(Packet));
	}

	void Deserialize(char* data) {
		memcpy(this, data, sizeof(Packet));
	}
};