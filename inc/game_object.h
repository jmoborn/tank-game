#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glut.h>
#endif

#include <iostream>
#include <string>
#include "vec4.h"
#include "mesh.h"

class game_object{
public:
	game_object(int id, int geo_id, int tex_id, int health=1);
	~game_object();
	int health;
	int id;
	int geo;
	int tex;

};

#endif

