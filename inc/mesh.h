#ifndef __MESH_H__
#define __MESH_H__

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>
#include "vec4.h"

class face {
public:
	face(std::vector<int> pts, std::vector<int> txts);
	~face(){};

	std::vector<int> pnts;
	std::vector<int> txts;

private:
	bool isQuad;
};

class mesh {
public:
	mesh();
	mesh(std::string filepath);
	~mesh(){};

	std::vector<vec4> verts;
	std::vector<face> faces;
	std::vector<vec2> texts;

	void readobj(std::string& filepath);

	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double zmin;
	double zmax;
	double rad;
};

#endif

