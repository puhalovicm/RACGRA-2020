#include <stdlib.h>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>
#include <vector>
#include "glm/vec3.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <ctime> 
#include <chrono>
#include "SOIL.h"

using namespace std::chrono;

#define PI 3.14159265

char *filename = (char *)"iskrica.tga";

float period = 15000;

bool animate = false;
bool alpha = false;
bool sizeChange = false;
bool gravity = false;

glm::vec3 source = glm::vec3(0, 0, 0);

glm::vec3 source2 = glm::vec3(-5, 0, -5);


bool click = true;

glm::vec3 cameraDir;

glm::vec3 startPos;
glm::vec3 startDir;

glm::vec3 eye;
glm::vec3 target;

GLuint window;
GLuint width = 300, height = 300;

int refreshMills = 10;
float t_o = 0;
int i_o = 1;

int mouseXlast = 0;
int mouseYlast = 0;

int mouseXnew = 0;
int mouseYnew = 0;

GLuint texture;
unsigned char *image;

struct Particle {
	glm::vec3 Position, Velocity;
	glm::vec2 Size;
	glm::vec4 Color;
	GLfloat Life;
	time_t StartTime;

	Particle() {}
};

GLuint nr_particles = 100;
std::vector<Particle> particles;
std::vector<Particle> particles2;


void myReshape(int width, int height);
void OnDraw();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void timer(int value);
void passiveMouse(int x, int y);
void onMouseButton(int button, int state, int x, int y);
void drawTexturedRect(float x, float y, float z, float w, float h, float a, GLuint texture);
void loadTexture(GLuint* texture, char* path, unsigned char *);
void spawnParticle(Particle &particle, time_t time, glm::vec3 source);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);

	window = glutCreateWindow("LV1");

	glutDisplayFunc(OnDraw);
	glutReshapeFunc(myReshape);
	glutKeyboardFunc(myKeyboard);
	glutTimerFunc(0, timer, 0);
	glutMotionFunc(passiveMouse);
	glutMouseFunc(onMouseButton);

	eye = glm::vec3(0, 15, 15);
	target = glm::vec3(0, 0, 0);
	cameraDir = glm::normalize(target - eye);

	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);

	for (GLuint i = 0; i < nr_particles; ++i) {
		Particle p = Particle();
		spawnParticle(p, ms.count(), source);
		particles.push_back(p);

		p = Particle();
		spawnParticle(p, ms.count(), source2);
		particles2.push_back(p);
	}

	loadTexture(&texture, filename, image);

	glutMainLoop();
	return 0;
}

void myReshape(int w, int h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();
	gluPerspective(45, (float)w / h, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
}

void spawnParticle(Particle &particle, time_t time, glm::vec3 source)
{
	GLfloat randomDir = ((rand() % 100) - 50) / 10.0f;

	particle.Velocity.x = randomDir;
	randomDir = ((rand() % 100) - 50) / 10.0f;

	particle.Velocity.z = randomDir;
	particle.Velocity.y = 10.0f;
	particle.Position = source;

	particle.Color.x = 1.0f;
	particle.Color.y = 1.0f;
	particle.Color.z = 1.0f;
	particle.Color.a = 1.0f;

	particle.Life = rand() % 3 + 0.5f;
	particle.StartTime = time;
	particle.Size = glm::vec2(1.0f, 1.0f);
} 

void OnDraw()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	float diffX = (mouseXnew - mouseXlast) * 0.005f;
	float diffY = (mouseYnew - mouseYlast) * 0.005f;

	float dc = glm::length(target - eye);

	glm::mat3 mx = glm::mat3(cos(diffX), 0, sin(diffX), 0, 1, 0, -sin(diffX), 0, cos(diffX));
	glm::mat3 my = glm::mat3(1, 0, 0, 0, cos(diffY), sin(diffY), 0, -sin(diffY), cos(diffY));

	cameraDir = mx * my * cameraDir;
	target = eye + dc * cameraDir;

	mouseXlast = mouseXnew;
	mouseYlast = mouseYnew;

	// set the camera position
	gluLookAt(eye.x, eye.y, eye.z,	//	eye pos
		target.x, target.y, target.z,	//	aim point
		0, 1, 0);	//	up direction

	milliseconds ms = duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()
		);

	time_t curr = ms.count();

	// Draw a grid
	glBegin(GL_LINES);
	for (int i = -10; i <= 10; i++) {
		glVertex3f(i, 0, 10);
		glVertex3f(i, 0, -10);

		glVertex3f(10, 0, i);
		glVertex3f(-10, 0, i);
	}
	glEnd();


	for (GLuint i = 0; i < nr_particles; ++i)
	{
		Particle &p = particles[i];
		
		float dt = (curr - p.StartTime) / period;

		p.Life -= dt; // reduce life
		if (sizeChange) p.Size.x -= 0.5f * dt; // reduce life
		if (sizeChange) p.Size.y -= 0.5f * dt; // reduce life
		if (p.Life > 0.0f)
		{	
			p.Position += p.Velocity * dt;
			if (gravity) p.Velocity.y -= 0.2f;
			if (alpha) p.Color.a -= dt * 0.5;
		}
		else {
			spawnParticle(p, curr, source);
		}
		drawTexturedRect(p.Position.x, p.Position.y, p.Position.z, p.Size.x, p.Size.y, p.Color.a, texture);
	}

	for (GLuint i = 0; i < nr_particles; ++i)
	{
		Particle &p = particles2[i];

		float dt = (curr - p.StartTime) / period;

		p.Life -= dt; 
		if (sizeChange) p.Size.x -= 0.5f * dt; 
		if (sizeChange) p.Size.y -= 0.5f * dt; 
		if (p.Life > 0.0f)
		{	
			p.Position += p.Velocity * dt;

			if (gravity) p.Velocity.y -= 0.2f;
			if (alpha) p.Color.a -= dt * 0.5;
		}
		else {
			spawnParticle(p, curr, source2);
		}
		drawTexturedRect(p.Position.x, p.Position.y, p.Position.z, p.Size.x, p.Size.y, p.Color.a, texture);
	}

	glutSwapBuffers();
}


void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
	float angle = 0.002f;
	float dc = glm::length(target - eye);

	glm::vec3 left = -glm::cross(cameraDir, glm::vec3(0, 1, 0));
	glm::vec3 right = -left;

	switch (theKey)
	{
	case 27:  exit(0);
		break;

	case 'q':
		eye.y = eye.y + 1;
		break;

	case 'e':
		eye.y = eye.y - 1;
		break;

	case 'a':
		eye += left;
		target = eye + dc * cameraDir;

		break;

	case 'd':
		eye += right;
		target = eye + dc * cameraDir;

		break;

	case 'w':
		eye += cameraDir;
		target += cameraDir;
		break;

	case 's':
		eye -= cameraDir;
		target -= cameraDir;
		break;

	case 'p':
		animate = !animate;
		break;

	case 'j':
		source.x -= 0.5f;

		break;

	case 'l':
		source.x += 0.5f;

		break;

	case 'k':
		source.z += 0.5f;

		break;

	case 'i':
		source.z -= 0.5f;

		break;

	case '1':
		gravity = !gravity;
		break;

	case '2':
		alpha = !alpha;
		break;

	case '3':
		sizeChange = !sizeChange;
		break;
	}

	cameraDir = glm::normalize(target - eye);

	glutPostRedisplay();
}

void timer(int value) {
	glutPostRedisplay();    
	glutTimerFunc(refreshMills, timer, 0); 
}

void passiveMouse(int x, int y)
{
	mouseXnew = x;
	mouseYnew = y;

	if (click) {
		mouseXlast = x;
		mouseYlast = y;
		click = false;
	}
}

void onMouseButton(int button, int state, int x, int y)
{
	click = true;
}

void loadTexture(GLuint* texture, char* path, unsigned char* image) {
	*texture = SOIL_load_OGL_texture(path,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS
	);
	int w = 1, h = 1;

	if (*texture == NULL) {
		printf("[Texture loader] \"%s\" failed to load!\n", path);
	}
}

void drawTexturedRect(float x, float y, float z, float w, float h, float a, GLuint texture) {	
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture);

	float modelview[16];
	float projection[16];
	int i, j;

	glPushMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);

	glColor4f(1, 1, 1, a);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(x, y, z);
	glTexCoord2f(0, 1); glVertex3f(x + w, y, z);
	glTexCoord2f(1, 1); glVertex3f(x + w, y + h, z);
	glTexCoord2f(1, 0); glVertex3f(x , y + h, z);
	glEnd();

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glDisable(GL_TEXTURE_2D);
}
