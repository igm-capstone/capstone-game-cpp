#pragma once
#include <string.h>

#define DEFAULT_PORT "7895" 
#define MAX_DATA_SIZE 1000000

enum PacketTypes {
	INIT_CONNECTION = '0',
	SET_CLIENT_ID	= '1',
	SPAWN_EXPLORER	= 'E',
	SPAWN_SKILL		= 'S',
	GRANT_AUTHORITY = 'A',
	SYNC_TRANSFORM	= 'T',
	SYNC_HEALTH		= 'H',
	SYNC_ANIMATION	= 'N',
	INTERACT		= 'I',
	READY			= 'R',
	DISCONNECT		= 'D',
	UNKNOWN
};

enum SkillPacketTypes : char
{
	SKILL_TYPE_UNKNOWN,
	SKILL_TYPE_HEAL,
	SKILL_TYPE_POISON,
	SKILL_TYPE_SLOW,
	SKILL_TYPE_LANTERN,
	SKILL_TYPE_IMP_MINION,
	SKILL_TYPE_ABOMINATION_MINION,
	SKILL_TYPE_FLYTRAP_MINION,
	SKILL_TYPE_EXPLOSION,
};

struct Packet {
	PacketTypes Type = UNKNOWN;
	
	union {
		unsigned int UUID = 0;
	}; //Target GO 

	union {
		unsigned int ClientID = 0;
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
			SkillPacketTypes Type = SKILL_TYPE_UNKNOWN;
		} AsSkill;

		float AsFloatArray[7];

		float AsFloat;
		bool AsBool;
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