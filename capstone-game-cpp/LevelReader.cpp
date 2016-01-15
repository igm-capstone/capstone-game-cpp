#include "stdafx.h"
#include "LevelReader.h"

using namespace Rig3D;

// Constructor and Destructor
LevelReader::LevelReader(std::string _path)
{
	mPath = _path;
}

LevelReader::~LevelReader()
{

}

// Member functions
void LevelReader::ReadWalls(std::string path, RectInfo *WallArray)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);
	// Go to Walls section
	while (std::getline(lvlFile,readLine))
	{
		if (readLine == "WALLS:") break;
	}
	// Go to Positions section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	while (std::getline(lvlFile, readLine))
	{
		//Iterate til the Edn of data section
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
		float x, y, z;
		iss >> x >> y >> z;
		vec3f* insertVec = new vec3f(x, y, z);
		WallArray->Position.insert(WallArray->Position.end(), *insertVec);
		}
	}

	// Go to Rotation section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Rotation") break;
	}

	// Read Rotaion line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			WallArray->Rotation.insert(WallArray->Rotation.end(), *insertVec);
		}
	}

	// Go to Scale section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Scale") break;
	}

	// Read Scale line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			WallArray->Scale.insert(WallArray->Scale.end(), *insertVec);
		}
	}
	// Close file
	lvlFile.close();
}

void LevelReader::ReadBlocks(std::string path, RectInfo *BlockArray)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);
	// Go to Blocks section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "MOVEABLEOBJECTS:") break;
	}
	// Go to Positions section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	while (std::getline(lvlFile, readLine))
	{
		//Iterate til the Edn of data section
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			BlockArray->Position.insert(BlockArray->Position.end(), *insertVec);
		}
	}

	// Go to Rotation section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Rotation") break;
	}

	// Read Rotaion line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			BlockArray->Rotation.insert(BlockArray->Rotation.end(), *insertVec);
		}
	}

	// Go to Scale section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Scale") break;
	}

	// Read Scale line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			BlockArray->Scale.insert(BlockArray->Scale.end(), *insertVec);
		}
	}
	// Close file
	lvlFile.close();
}

void LevelReader::ReadPlayer(std::string path, vec3f *PlayerPos)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);
	// Go to Blocks section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "PLAYER:") break;
	}
	// Go to Positions section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	std::getline(lvlFile, readLine);
	std::istringstream iss(readLine);
	if (iss)
	{
		float x, y, z;
		iss >> x >> y >> z;
		*PlayerPos = vec3f(x, y, z);
	}
	// Close file
	lvlFile.close();
}

void LevelReader::ReadRobot(std::string path, std::vector<RobotInfo> *RobotArray)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);
	// Go to Robot section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "ROBOT:") break;
	}

	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "LIGHT:") break;

		RobotInfo InsertRobot;
		if (readLine == "Position")
		{
			// Read position Data
			std::getline(lvlFile, readLine);
			std::istringstream iss(readLine);
			if (iss)
			{
				float x, y, z;
				iss >> x >> y >> z;
				InsertRobot.Transform.SetPosition(x, y, z);
			}
		}
		// Read next line
		std::getline(lvlFile, readLine);
		if (readLine == "WAYPOINTS")
		{
			while (std::getline(lvlFile, readLine))
			{
				if (readLine == "END") break;
				std::istringstream iss(readLine);
				if (iss)
				{
					float x, y, z;
					iss >> x >> y >> z;
					vec3f insertVec = vec3f(x, y, z);
					InsertRobot.Waypoints.insert(InsertRobot.Waypoints.end(), insertVec);
				}
			}
		}
		RobotArray->insert(RobotArray->end(), InsertRobot);
	}
	// Close file
	lvlFile.close();
}

void LevelReader::ReadLights(std::string path, std::vector<vec3f> *LightArray)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);

	// Go to Lights section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "LIGHT:") break;
	}
	// Go to Positions section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	while (std::getline(lvlFile, readLine))
	{
		//Iterate til the Edn of data section
		if (readLine == "END") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			LightArray->insert(LightArray->end(), *insertVec);
		}
	}
	// Close file
	lvlFile.close();
}

void LevelReader::ReadGoal(std::string path, vec3f *GoalPos)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Open file
	lvlFile.open(path);
	// Go to Blocks section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "GOAL:") break;
	}
	// Go to Positions section
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	std::getline(lvlFile, readLine);
	std::istringstream iss(readLine);
	if (iss)
	{
		float x, y, z;
		iss >> x >> y >> z;
		*GoalPos = vec3f(x, y, z);
	}
	// Close file
	lvlFile.close();
}


void LevelReader::ReadLevel()
{
	// Call every single read file function
	ReadWalls(mPath, &mWalls);
	ReadBlocks(mPath, &mBlocks);
	ReadPlayer(mPath, &mPlayerPos);
	ReadRobot(mPath, &mRobots);
	ReadLights(mPath, &mLights);
	ReadGoal(mPath, &mGoalPos);
}

