#include "LevelReader.h"

using namespace Rig3D;

void readWalls(std::string path)
{
	// Declarations
	// File stream reader
	std::fstream lvlFile;
	std::string readLine;

	// Wall Arrays
	std::vector<vec3f> WallPos, WallRot, WallScale;

	// Open file
	lvlFile.open(path);
	// Go to Walls section
	while (std::getline(lvlFile,readLine))
	{
		if (readLine == "WALLS: ") break;
	}
	
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read Positions line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Rotation") break;
		std::istringstream iss(readLine);
		if (iss)
		{
		float x, y, z;
		iss >> x >> y >> z;
		vec3f* insertVec = new vec3f(x, y, z);
		WallPos.insert(WallPos.end(), *insertVec);
		}
	}

	// Trow last vector out because it gets repeated
	WallPos.erase(WallPos.end());

	// Read Rotaion line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Scale") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			WallRot.insert(WallPos.end(), *insertVec);
		}
	}

	// Trow last vector out because it gets repeated
	WallRot.erase(WallPos.end());

	// Read Scale line from file
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "MOVEABLEOBJECTS: ") break;
		std::istringstream iss(readLine);
		if (iss)
		{
			float x, y, z;
			iss >> x >> y >> z;
			vec3f* insertVec = new vec3f(x, y, z);
			WallScale.insert(WallPos.end(), *insertVec);
		}
	}

	WallScale.erase(WallPos.end());
}
