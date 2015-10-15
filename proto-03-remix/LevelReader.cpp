#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Rig3D\Engine.h"
#include "Rig3D\GraphicsMath\cgm.h"

using namespace Rig3D;

void readWalls(std::string path)
{
	// Declarations
	// File stream reader
	std::ifstream lvlFile;
	std::string readLine;
	

	// Wall Arrays
	std::vector<vec3f> WallPos, WallRot, WallScale;

	// Open file
	lvlFile.open(path);
	// Go to Walls section
	while (std::getline(lvlFile,readLine))
	{
		if (readLine == "WALLS:") break;
	}
	
	while (std::getline(lvlFile, readLine))
	{
		if (readLine == "Position") break;
	}
	// Read line from file
	while (std::getline(lvlFile, readLine))
	{
		std::istringstream iss(readLine);
		float x, y, z;
	}

	// Switch case by each object type and subtype.


	


}
