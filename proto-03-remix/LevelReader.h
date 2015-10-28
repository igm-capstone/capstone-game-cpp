#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Rig3D\Engine.h"
#include "Rig3D\GraphicsMath\cgm.h"
#include <Rig3D/Common/Transform.h>

struct WallInfo
{
	std::vector<vec3f>	Position;
	std::vector<vec3f>	Rotation;
	std::vector<vec3f>	Scale;
};

struct BlockInfo
{
	std::vector<vec3f>	Position;
	std::vector<vec3f>	Rotation;
	std::vector<vec3f>	Scale;
};

struct RobotInfo
{
	Rig3D::Transform	Transform;
	std::vector<vec3f>	Waypoints;
};


class LevelReader
{

public:

	std::string				mPath;
	WallInfo				mWalls;
	BlockInfo				mBlocks;
	
	vec3f					mPlayerPos;
	vec3f					mGoalPos;
	
	std::vector<vec3f>		mLights;
	std::vector<RobotInfo>	mRobots;

	
	LevelReader(std::string _path);
	~LevelReader();

	void ReadLevel();

private:

	void ReadWalls(std::string path, WallInfo *WallArray);
	void ReadBlocks(std::string path, BlockInfo *BlockArray);
	void ReadPlayer(std::string path, vec3f *PlayerPos);
	void ReadRobot(std::string path, std::vector<RobotInfo> *RobotArray);
	void ReadLights(std::string path, std::vector<vec3f> *LightArray);
	void ReadGoal(std::string path, vec3f *GoalPos);

};

