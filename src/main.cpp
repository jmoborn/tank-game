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
#include <math.h>
#include <vector>
#include <stddef.h>
#include <time.h>

#include "vec4.h"
#include "mat4.h"
#include "mesh.h"
#include "game_object.h"

#define KEYBOARD_S 115
#define KEYBOARD_ESC 27
#define KEYBOARD_F 102
#define NORMAL_EXIT_GLUT_LOOP "terminating glut"

int windowWidth = 640;
int windowHeight = 480;
bool specialKeys[1000] = {0};

std::vector<mesh> meshes;
std::vector<game_object*> characters;
std::vector<int> bad_guys;
game_object *camera; //DO NOT DRAW

int HERO_ID = 0;
int ENEMY_ID = 1;
int ENVIRONMENT_ID = 2;
int TARGET_ID = 3;
int SWIVEL_ID = 4;
int BARREL_ID = 5;
GLuint HERO_TEX = 0;
GLuint ENEMY_TEX = 1;
GLuint ENVIRONMENT_TEX = 2;
GLuint TARGET_TEX = 3;
GLuint SWIVEL_TEX = 4;
GLuint BARREL_TEX = 5;

double TANK_TURN = 1.0;
double TANK_SPEED = 0.1;
double TURRET_TURN = 1.0;
double TURRET_SPEED = .1;

double BAD_GUY_SPEED = 0.01;
double last_update;
double UPDATE_INC = 0.05;

int turret_rotate = 0;
int turret_translate = 0;
int MIN_TURRET_ROTATE = -30;
int MAX_TURRET_ROTATE = 30;
int MIN_TURRET_TRANSLATE = 7.0;
int MAX_TURRET_TRANSLATE = 11.0;

bool HandleKeyboardInput();
GLvoid InitGL();
GLvoid DrawGLScene();
GLvoid IdleGLScene();
GLvoid ResizeGLScene(int width, int height);
GLvoid GLKeyDown(unsigned char key, int x, int y);
GLvoid SpecialKeys(int key, int x, int y);
GLvoid SpecialKeysUp(int key, int x, int y);
int loadPPM(const char *filename, GLuint textureID);
void DrawObj(mesh& m, mat4& t);

/*
Function taken from www.cs.utexas.edu texture.cpp
*/
int loadPPM(const char *filename, GLuint textureID) {

	//GLuint textureID = 0; 
    FILE *inFile; //File pointer
	char buffer[100]; //Input buffer
    GLubyte *theTexture; //Texture buffer pointer
	unsigned char c; //Input character
	int width, height, maxVal, pixelSize; //Image characteristics from ppm file

	//Try to open the file for reading
	if( (inFile = fopen(filename, "rb")) == NULL) {
		fprintf (stderr, "cannot open %s\n", filename);
		exit(-1);
	}

	//Read file type identifier (magic number)
	fgets(buffer, sizeof(buffer), inFile);
	if ((buffer[0] != 'P') || (buffer[1] != '6')) {
		fprintf (stderr, "not a binary ppm file %s\n", filename);
		exit(-1);
    }

	if(buffer[2] == 'A')
		pixelSize = 4;
	else
		pixelSize = 3;

	//Read image size
	do fgets(buffer, sizeof (buffer), inFile);
	while (buffer[0] == '#');
	sscanf (buffer, "%d %d", &width, &height);

	//Read maximum pixel value (usually 255)
	do fgets (buffer, sizeof (buffer), inFile);
	while (buffer[0] == '#');
	sscanf (buffer, "%d", &maxVal);

	//Allocate RGBA texture buffer
	int memSize = width * height * 4 * sizeof(GLubyte);
	theTexture = (GLubyte *)malloc(memSize);

	// read RGB data and set alpha value
	for (int i = 0; i < memSize; i++) {
		if ((i % 4) < 3 || pixelSize == 4) {
			c = fgetc(inFile);
			theTexture[i]=(GLubyte) c;
        }
		else theTexture[i] = (GLubyte) 255; //Set alpha to opaque
    }
    fclose(inFile);

    glBindTexture(GL_TEXTURE_2D, textureID);

    //Set texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	//Ignore surface color
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //Define the texture
    glTexImage2D(GL_TEXTURE_2D, 0, 4, (GLuint)width, (GLuint)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, theTexture);

    //Create mipmaps
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, (GLuint)width, (GLuint)height, GL_RGBA, GL_UNSIGNED_BYTE, theTexture);

    free(theTexture);
    return(textureID);
}

GLvoid InitGL(){
	//All the stuff we need to draw
	std::string hero_geo_file = "geo/car.obj";
	std::string enemy_geo_file = "geo/tire.obj";
	std::string environment_geo_file = "geo/ParkingLot.obj";
	std::string target_geo_file = "geo/target.obj";
	std::string swivel_geo_file = "geo/swivel.obj";
	std::string barrel_geo_file = "geo/barrel.obj";

	std::string hero_tex_file = "tex/car.ppm";
	std::string enemy_tex_file = "tex/tire.ppm";
	std::string environment_tex_file = "tex/ParkingLot.ppm";
	std::string target_tex_file = "tex/target.ppm";
	std::string swivel_tex_file = "tex/swivel.ppm";
	std::string barrel_tex_file = "tex/barrel.ppm";

	mesh hero(hero_geo_file);
	mesh enemy(enemy_geo_file);
	mesh environment(environment_geo_file);
	mesh target(target_geo_file);
	mesh swivel(swivel_geo_file);
	mesh barrel(barrel_geo_file);
	HERO_ID = meshes.size();
	meshes.push_back(hero);
	ENEMY_ID = meshes.size();
	meshes.push_back(enemy);
	ENVIRONMENT_ID = meshes.size();
	meshes.push_back(environment);
	TARGET_ID = meshes.size();
	meshes.push_back(target);
	SWIVEL_ID = meshes.size();
	meshes.push_back(swivel);
	BARREL_ID = meshes.size();
	meshes.push_back(barrel);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &HERO_TEX);
	glGenTextures(1, &ENEMY_TEX);
	glGenTextures(1, &ENVIRONMENT_TEX);
	glGenTextures(1, &TARGET_TEX);
	glGenTextures(1, &SWIVEL_TEX);
	glGenTextures(1, &BARREL_TEX);
	loadPPM(hero_tex_file.c_str(), HERO_TEX);
	loadPPM(enemy_tex_file.c_str(), ENEMY_TEX);
	loadPPM(environment_tex_file.c_str(), ENVIRONMENT_TEX);
	loadPPM(target_tex_file.c_str(), TARGET_TEX);
	loadPPM(swivel_tex_file.c_str(), SWIVEL_TEX);
	loadPPM(barrel_tex_file.c_str(), BARREL_TEX);

	camera = new game_object(-1,-1,-1);

	game_object *hero_character = new game_object(characters.size(), HERO_ID, HERO_TEX);
	characters.push_back(hero_character);
	game_object *enemy_character = new game_object(characters.size(), ENEMY_ID, ENEMY_TEX);
	enemy_character->parent_to(camera);
	characters.push_back(enemy_character);
	game_object *environment_character = new game_object(characters.size(), ENVIRONMENT_ID, ENVIRONMENT_TEX);
	environment_character->parent_to(camera);
	characters.push_back(environment_character);
	game_object *target_character = new game_object(characters.size(), TARGET_ID, TARGET_TEX);
	target_character->transform.translate(0.0,0.0,-7.0);
	characters.push_back(target_character);
	game_object *swivel_character = new game_object(characters.size(), SWIVEL_ID, SWIVEL_TEX);
	swivel_character->transform.translate(0.0,0.4,0.0);
	characters.push_back(swivel_character);
	game_object *barrel_character = new game_object(characters.size(), BARREL_ID, BARREL_TEX);
	// barrel_character.transform.translate(0.0,0.4,0.0);
	barrel_character->parent_to(characters[SWIVEL_ID]);
	characters.push_back(barrel_character);
	bad_guys.push_back(ENEMY_ID);
	for(int i=0; i<10; i++)
	{
		game_object *enemyX = new game_object(characters.size(), ENEMY_ID, ENEMY_TEX);
		enemyX->transform.translate(i*1.0, 0.0, i*1.0);
		enemyX->parent_to(camera);
		bad_guys.push_back(characters.size());
		characters.push_back(enemyX);
		bad_guys.push_back(ENEMY_ID);
	}
	// clock_t start = clock();
	// (double)time_gone_by / ((double)CLOCKS_PER_SEC)
	last_update = (double)clock() / ((double)CLOCKS_PER_SEC);

	//GL boilerplate initialization
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);				// grey Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing				
	
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glEnable(GL_COLOR_MATERIAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
} 

void DrawObj(mesh& m)
{
	glBegin(GL_TRIANGLES);
    for(int i=0; i<m.faces.size(); i++)
    {
    	vec4 vert = m.verts[m.faces[i].pnts[0]];
    	vec2 text = m.texts[m.faces[i].txts[0]];
    	glTexCoord2f(text.u, text.v);
    	glVertex3f(vert.x,vert.y,vert.z);
    	vert = m.verts[m.faces[i].pnts[1]];
    	text = m.texts[m.faces[i].txts[1]];
    	glTexCoord2f(text.u, text.v);
    	glVertex3f(vert.x,vert.y,vert.z);
    	vert = m.verts[m.faces[i].pnts[2]];
    	text = m.texts[m.faces[i].txts[2]];
    	glTexCoord2f(text.u, text.v);
    	glVertex3f(vert.x,vert.y,vert.z);
    }
	glEnd();	
}

GLvoid DrawGLScene(){

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0,-1.0,-3.25);
	// glRotatef(5.0,1,0,0);
    
    glViewport(0, 0, windowWidth, windowHeight);
    // glTranslatef(0.0f, 0.0f, -5.0f);

    for(int i=0; i<characters.size(); i++)
    {
    	glBindTexture(GL_TEXTURE_2D, characters[i]->tex);
    	glPushMatrix();
    	glMultMatrixd(characters[i]->get_transform());
    	DrawObj(meshes[characters[i]->geo]);
    	// glMultMatrixd(initial_transform.get_transform());
    	// if(i==BARREL_ID)
    	// {
    	// DrawObj(meshes[characters[HERO_ID].geo]);
    	// std::cout << "drew barrel" << std::endl;
    	// }
    	// else
    	// {
    	// DrawObj(meshes[characters[TARGET_ID].geo]);
    	// }
    	glPopMatrix();
    	// if(i==BARREL_ID)
    	// {
    	// for(int j=0; j<4; j++)
    	// {
    	// 	for(int k=0; k<4; k++)
    	// 	{
    	// 		std::cout << characters[i]->parent->transform(k,j) << " ";
    	// 	}
    	// 	std::cout << std::endl;
    	// }
    	// // 	std::cout << characters[i].transform(0,3) << characters[i].transform(1,3)<< characters[i].transform(2,3) << std::endl;
    	// }
    }

 //    double now = (double)clock() / ((double)CLOCKS_PER_SEC);
 //    if(now - last_update > UPDATE_INC)
 //    {
	//     for(int i=0; i<bad_guys.size(); i++)
	//     {
	//     	vec4 pos = characters[bad_guys[i]]transform*vec4(0,0,0);
	//     	pos.normalize();
	//     	characters[bad_guys[i]].transform.translate(-pos.x, -pos.y, -pos.z);
	//     }
	// }
    glFlush();
	glutSwapBuffers();
}

GLvoid IdleGLScene(){
	if(HandleKeyboardInput())
	{	
		glutPostRedisplay();
	}
	double now = (double)clock() / ((double)CLOCKS_PER_SEC);
    if(now - last_update > UPDATE_INC)
    {
	    for(int i=0; i<bad_guys.size(); i++)
	    {
	    	vec4 pos = characters[bad_guys[i]]->transform*vec4(0,0,0);
	    	pos.normalize();
	    	pos *= BAD_GUY_SPEED;
	    	characters[bad_guys[i]]->transform.translate(-pos.x, -pos.y, -pos.z);
	    }
	    glutPostRedisplay();
	}
}

GLvoid ResizeGLScene(int width, int height){
	if(height == 0)
		height = 1;
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 2000.0f);

	windowWidth = width;
	windowHeight = height;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

bool test_collisions()
{
	vec4 pos = characters[TARGET_ID]->transform*vec4(0,0,0);
	for(int i=0; i<bad_guys.size(); i++)
	{

	}
}

/*
 * This function handles all normal key presses on the keyboard. If you need
 * to capture special keys like, ctrl, shift, F1, F2, F..., or arrow keys use
 * the special keys function
 */
GLvoid GLKeyDown(unsigned char key, int x, int y){
	if(key==' ')
	{
		
	}
	if(key=='w')
	{
		vec4 pos = characters[TARGET_ID]->transform*vec4(0,0,0);
		double dist = pos.length();
		if(dist<MAX_TURRET_TRANSLATE)
		{
			vec4 mv_dir = (pos)*TURRET_SPEED;
			characters[TARGET_ID]->transform.translate(mv_dir.x, 0, mv_dir.z);
			characters[BARREL_ID]->transform.rotateX(2.0);
			glutPostRedisplay();
		}
	}
	if(key=='s')
	{
		vec4 pos = characters[TARGET_ID]->transform*vec4(0,0,0);
		double dist = pos.length();
		if(dist>MIN_TURRET_TRANSLATE)
		{
			vec4 mv_dir = (pos)*TURRET_SPEED;
			characters[TARGET_ID]->transform.translate(-mv_dir.x, 0, -mv_dir.z);
			characters[BARREL_ID]->transform.rotateX(-2.0);
			glutPostRedisplay();
		}
	}
	if(key=='a'&&turret_rotate<MAX_TURRET_ROTATE)
	{
		turret_rotate++;
		characters[TARGET_ID]->transform.rotateY(TURRET_TURN);
		characters[SWIVEL_ID]->transform.rotateY(TURRET_TURN);
		glutPostRedisplay();
	}
	if(key=='d'&&turret_rotate>MIN_TURRET_ROTATE)
	{
		turret_rotate--;
		characters[TARGET_ID]->transform.rotateY(-TURRET_TURN);
		characters[SWIVEL_ID]->transform.rotateY(-TURRET_TURN);
		glutPostRedisplay();
	}
}

/*
 * These are the special keys as is set apart by glut
 */
GLvoid SpecialKeys(int key, int x, int y){
	switch(key){
		case GLUT_KEY_LEFT:
			specialKeys[GLUT_KEY_LEFT] = 1;
			break;
		case GLUT_KEY_RIGHT:
			specialKeys[GLUT_KEY_RIGHT] = 1;
			break;
		case GLUT_KEY_UP:
			specialKeys[GLUT_KEY_UP] = 1;
			break;
		case GLUT_KEY_DOWN:
			specialKeys[GLUT_KEY_DOWN] = 1;
			break;
		default:
			break;
	}
}

GLvoid SpecialKeysUp(int key, int x, int y){
	switch(key){
		case GLUT_KEY_LEFT:
			specialKeys[GLUT_KEY_LEFT] = 0;
			break;
		case GLUT_KEY_UP:
			specialKeys[GLUT_KEY_UP] = 0;
			break;
		case GLUT_KEY_RIGHT:
			specialKeys[GLUT_KEY_RIGHT] = 0;
			break;
		case GLUT_KEY_DOWN:
			specialKeys[GLUT_KEY_DOWN] = 0;
			break;
		default:
			break;
	}
}

bool HandleKeyboardInput(){
	if(specialKeys[GLUT_KEY_LEFT]){
		camera->transform.rotateY(-TANK_TURN);
		return true;
	}
	if(specialKeys[GLUT_KEY_RIGHT]){
		camera->transform.rotateY(TANK_TURN);
		return true;
	}
	if(specialKeys[GLUT_KEY_UP]){
		camera->transform.translate(0.0,0.0,TANK_SPEED);
		return true;
	}
	if(specialKeys[GLUT_KEY_DOWN]){
		camera->transform.translate(0.0,0.0,-TANK_SPEED);
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	char windowName[] = "TANK X";
	int windowWidth = 640;
	int windowHeight = 480;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(windowName);

	InitGL();
	glutDisplayFunc(DrawGLScene);
	glutReshapeFunc(ResizeGLScene);
	glutIdleFunc(IdleGLScene);
	glutKeyboardFunc(GLKeyDown);
	glutSpecialFunc(SpecialKeys);
	glutSpecialUpFunc(SpecialKeysUp);
	glutMainLoop();
}
