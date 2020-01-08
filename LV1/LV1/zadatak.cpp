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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define PI 3.14159265

bool animate = false;

bool click = true;

glm::vec3 cameraDir;

int k = 3;
std::vector<glm::vec3> r;
std::vector<int> Uknot;
int n;

glm::vec3 startPos;
glm::vec3 startDir;

std::vector<glm::vec4> vertexList;
std::vector<glm::vec4> faceList;

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

void myReshape(int width, int height);
void OnDraw();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void timer(int value);
glm::vec3 calculateVec(float t, int i);
glm::vec3 calculateVecD(float t, int i);
void passiveMouse(int x, int y);
void onMouseButton(int button, int state, int x, int y);

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

	std::string filename;
	std::ifstream infile;

	do {
		std::cout << "ime datoteke>>";
		std::cin >> filename;

		infile.open(filename);
	} while (infile.fail());

	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		char ch;
		if (!(iss >> ch)) {
			for (int i = 0; i < line.length(); i++)
			{
				if (line.at(i) != ' ')
				{
					break;
				}
			}
			continue;
		}

		if (ch == 'v')
		{
			double a, b, c;

			if (!(iss >> a >> b >> c)) {
				break;
			}

			vertexList.push_back(glm::vec4(a, b, c, 1));
		}
		else if (ch == 'f')
		{
			int a, b, c;
			if (!(iss >> a >> b >> c)) {
				break;
			}
			faceList.push_back(glm::vec4(a, b, c, 1));
		}
		else if (ch == '#')
		{
			continue;
		}
	}

	std::ifstream infilestream;
	std::string line2;

	infilestream.open("tocke.txt");
	while (infilestream)
	{
		std::getline(infilestream, line2);

		std::istringstream iss(line2);
		double a, b, c;

		if (!(iss >> a >> b >> c)) {
			break;
		}

		r.push_back(glm::vec3(a, b, c));
	}
	infilestream.close();

	n = r.size();

	eye = glm::vec3(28, -2, -5);
	target = glm::vec3(0, 0, 0);
	cameraDir = glm::normalize(target - eye);

	startPos = r.at(0);
	startDir = r.at(n - 1) - r.at(0);

	vertexList.push_back(vertexList.at(0));

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
	gluLookAt(eye.x, eye.y, eye.z,
		target.x, target.y, target.z,	
		0, 1, 0);	

	int gridSize = 100;

	glColor3f(0.5f, 0.5f, 0.5f);
	// Draw a grid
	glBegin(GL_LINES);
	for (int i = -gridSize; i <= gridSize; i++) {
		glVertex3f(i, 0, gridSize);
		glVertex3f(i, 0, -gridSize);

		glVertex3f(gridSize, 0, i);
		glVertex3f(-gridSize, 0, i);
	}
	glEnd();

	glColor3f(1, 1, 0);
	glBegin(GL_LINE_STRIP);
	for (int i = 1; i <= n - 3; i++) 
	{
		// use the parametric time value 0 to 1
		for (float t = 0; t < 1; t += 0.01f) 
		{
			glm::vec3 v = calculateVec(t, i);

			// specify the point
			glVertex3f(v.x, v.y, v.z);
		}
	}
	glEnd();

	glPushMatrix();

	glm::vec3 v_o = calculateVec(t_o, i_o);
	glTranslatef(v_o.x, v_o.y, v_o.z);

	glm::vec3 vd_o = calculateVecD(t_o, i_o);
	
	glm::vec3 os = glm::cross(startDir, vd_o);

	float cosAngle = glm::dot(startDir, vd_o) / (glm::length(startDir) * glm::length(vd_o));
	float angle = acos(cosAngle) * 180.0 / PI;

	glRotatef(angle, os.x, os.y, os.z);

	glColor3f(1.0f, 1.0, 1.0f);

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < faceList.size(); i++) {
		glm::vec4 v1 = vertexList[faceList[i].x - 1];
		glm::vec4 v2 = vertexList[faceList[i].y - 1];
		glm::vec4 v3 = vertexList[faceList[i].z - 1];
		glVertex3f(v1.x, v1.y, v1.z);
		glVertex3f(v2.x, v2.y, v2.z);
		glVertex3f(v3.x, v3.y, v3.z);
	}
	glEnd();

	glPopMatrix();

	glm::vec3 endP = v_o + 0.5f * vd_o;

	glColor3f(0.7f, 0, 0.8f);
	glBegin(GL_LINE_STRIP);
	glVertex3f(v_o.x, v_o.y, v_o.z);
	glVertex3f(endP.x, endP.y, endP.z);
	glEnd();

	// draw the control points
	glColor3f(0, 1, 0);
	glPointSize(3);
	glBegin(GL_POINTS);
	for (int i = 0; i < n; i++) {
		glVertex3f(r[i].x, r[i].y, r[i].z);
	}
	glEnd();

	// draw the hull of the curve
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < n; i++) {
		glVertex3f(r[i].x, r[i].y, r[i].z);
	}
	glEnd();

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

	case 'f':
		t_o -= 0.1;

		if (t_o < 0) {
			t_o += 1;
			i_o--;

			if (i_o < 1) {
				i_o = n - 3;
			}
		}
		break;
	case 'g':
		t_o += 0.1;
		if (t_o >= 1) {
			t_o -= 1;
			i_o++;

			if (i_o > n - 3) {
				i_o = 1;
			}
		}
		break;
	}

	cameraDir = glm::normalize(target - eye);

	glutPostRedisplay();
}

glm::vec3 calculateVec(float t, int i) {
	float b0 = (-t * t*t + 3 * t*t - 3 * t + 1) / 6.0f;
	float b1 = (3 * t*t*t - 6 * t*t + 4) / 6.0f;
	float b2 = (-3 * t*t*t + 3 * t*t + 3 * t + 1) / 6.0f;
	float b3 = t * t*t / 6.0f;

	float x = b0 * r[i - 1].x +
		b1 * r[i].x +
		b2 * r[i + 1].x +
		b3 * r[i + 2].x;

	float y = b0 * r[i - 1].y +
		b1 * r[i].y +
		b2 * r[i + 1].y +
		b3 * r[i + 2].y;

	float z = b0 * r[i - 1].z +
		b1 * r[i].z +
		b2 * r[i + 1].z +
		b3 * r[i + 2].z;

	return glm::vec3(x, y, z);
}

glm::vec3 calculateVecD(float t, int i) {
	float bd0 = (-t * t + 2 * t - 1) / 2.0f;
	float bd1 = (3 * t*t - 4 * t) / 2.0f;
	float bd2 = (-3 * t*t + 2 * t + 1) / 2.0f;
	float bd3 = t * t / 2.0f;

	float xd = bd0 * r[i - 1].x +
		bd1 * r[i].x +
		bd2 * r[i + 1].x +
		bd3 * r[i + 2].x;

	float yd = bd0 * r[i - 1].y +
		bd1 * r[i].y +
		bd2 * r[i + 1].y +
		bd3 * r[i + 2].y;

	float zd = bd0 * r[i - 1].z +
		bd1 * r[i].z +
		bd2 * r[i + 1].z +
		bd3 * r[i + 2].z;

	return glm::vec3(xd, yd, zd);
}

void timer(int value) {
	glutPostRedisplay();      
	glutTimerFunc(refreshMills, timer, 0); 

	if (animate) {
		t_o += 0.01;

		if (t_o >= 1) {
			t_o = 0;
			i_o++;

			if (i_o > n - 3) {
				i_o = 1;
			}
		}
	}
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