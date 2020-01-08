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
#include <GL/glut.h>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <map> 
#include <stdio.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/cimport.h>

int k = 3;
std::vector<glm::vec3> r;
std::vector<int> Uknot;
int n;

float t_z_values[16] = { -1, 3, -3, 3, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0 };
glm::mat4x4 Bi3 = glm::make_mat4(t_z_values);

glm::vec3 I;
glm::vec3 color = glm::vec3(255, 0, 0);

glm::vec3 Ia = glm::vec3(32, 0, 32);
double ka = 0.5;

double kd = 1;

// ociste
glm::vec3 eye;
// glediste
glm::vec3 target;
// viewUp
glm::vec3 viewUp;

glm::mat4x4 t1;

glm::vec3 z;
glm::vec3 x;
glm::vec3 y;

glm::mat4 t;
glm::mat4 p;

std::vector<glm::vec4> vertexList;
std::vector<glm::vec4> faceList;

int angle = 0;
float H = 1;

glm::mat4 t2;
glm::mat4 s;

glm::mat4 t_z;

bool myImpl = false;
bool paint = true;
bool gourad = false;

GLuint window;
GLuint width = 300, height = 300;

void myDisplay();
void myReshape(int width, int height);
void myMouse(int button, int state, int x, int y);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void myObject();
void redisplay_all(void);
void calculateTransformation();
void updatePerspective();
void calculateMinimax();
double baseFunction(int u, int k, int i);
void calculateUknot();
glm::vec4 segment(int i2, double t2);

int main(int argc, char** argv)
{
	// postavljanje dvostrukog spremnika za prikaz (zbog titranja)
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);

	window = glutCreateWindow("Tijelo");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);
	printf("Tipka: d - pomicanje ocista po x os +\n");
	printf("Tipka: a - pomicanje ocista po x os -\n");
	printf("Tipka: w - pomicanje ocista po y os +\n");
	printf("Tipka: s - pomicanje ocista po y os -\n");
	printf("Tipka: q - pomicanje ocista po z os +\n");
	printf("Tipka: e - pomicanje ocista po z os -\n");
	printf("Tipka: r - pocetno stanje\n");
	printf("esc: izlaz iz programa\n");

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
		else if (ch == 'G')
		{
			double a, b, c;

			if (!(iss >> a >> b >> c)) {
				break;
			}

			target = glm::vec3(a, b, c);
		}
		else if (ch == 'O')
		{
			double a, b, c;

			if (!(iss >> a >> b >> c)) {
				break;
			}

			eye = glm::vec3(a, b, c);
		}
		else if (ch == 'U')
		{
			double a, b, c;

			if (!(iss >> a >> b >> c)) {
				break;
			}

			viewUp = glm::vec3(a, b, c);
		}
		else
		{
		}

	}

	target = glm::vec3(0, 0, 0);
	eye = glm::vec3(1, 1, 1);
	viewUp = glm::vec3(0, 1, 0);

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

	std::cout << "n " << n;

	vertexList.push_back(vertexList.at(0));

	viewUp = glm::normalize(viewUp);
	calculateTransformation();

	calculateMinimax();

	double ix, iy, iz;

	//std::cout << "Unesi kordinate izvora I>>";
	//std::cin >> ix >> iy >> iz;

	ix = 0;
	iy = 1;
	iz = 0;

	I = glm::vec3(ix, iy, iz);

	glutMainLoop();
	return 0;
}

glm::vec3 getNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	glm::vec3 a = v2 - v1;
	glm::vec3 b = v3 - v1;
	return glm::cross(a, b);
}

glm::vec3 getCenter(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
	glm::vec3 center = glm::vec3(
		(v1.x + v2.x + v3.x) / 3,
		(v1.y + v2.y + v3.y) / 3,
		(v1.z + v2.z + v3.z) / 3);
	return center;
}

void myDisplay(void)
{
	// printf("Pozvan myDisplay()\n");
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		         // boja pozadine - bijela
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	myObject();
	glutSwapBuffers();      // iscrtavanje iz dvostrukog spemnika (umjesto glFlush)
}

void myReshape(int w, int h)
{
	// printf("MR: width=%d, height=%d\n",w,h);
	width = w; height = h;
	glViewport(0, 0, width, height);
	updatePerspective();

	glColor3f(1.0f, 0.5f, 0.5f);
	glBegin(GL_TRIANGLES);
	{
		glVertex3f(1, 1, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 1);
		glVertex3f(0, 1, 0);

		for (int i2 = 1; i2 <= n - 3; i2++) {
			for (double t2 = 0; t2 <= 1; t2 += 0.001)
			{
				glm::vec4 p = segment(i2, t2);

				//std::cout << "a";
				//std::cout << glm::to_string(p) << std::endl;

				glVertex3f(p.x, p.y, p.z);
			}
		}
	}
	glEnd();
}

void updatePerspective()
{
	glMatrixMode(GL_PROJECTION);        // aktivirana matrica projekcije
	glLoadIdentity();
	
	if (myImpl) 
	{
		glOrtho(-1.5 * (float)width / height , 1.5 * (float)width / height, 
			-1.5 * (float)height / width, 1.5 * (float)height / width, -50, 50);
	}
	else
	{
		gluPerspective(45.0, (float)width / height, 1.5, 50.0); // kut pogleda, x/y, prednja i straznja ravnina odsjecanja
	}

	glMatrixMode(GL_MODELVIEW);         // aktivirana matrica modela
	glLoadIdentity();
	if (!myImpl) 
	{
		gluLookAt(eye.x, eye.y, eye.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);	// ociste x,y,z; glediste x,y,z; up vektor x,y,z
	}
}

void myObject()
{
	//glutWireCube(1.0);
	//glutSolidCube (1.0);
	//glutWireTeapot (1.0);
	//glutSolidTeapot (1.0);
	/*
	glBegin(GL_TRIANGLES); // ili glBegin (GL_LINE_LOOP); za zicnu formu
	glColor3ub(255, 0, 0);	glVertex3f(-1.0, 0.0, 0.0);
	glColor3ub(0, 0, 0);	glVertex3f(0.0, 1.0, 0.0);
	glColor3ub(100, 0, 0);	glVertex3f(0.0, 0.0, 1.0);
	glEnd();*/

	calculateTransformation();

	std::vector<std::vector<glm::vec3>> vertexNormalArray(vertexList.size());
	std::vector<glm::vec3> vertexNormals;

	if (gourad)
	{
		for (int i = 0; i < faceList.size(); i++) {
			glm::vec4 v1;
			glm::vec4 v2;
			glm::vec4 v3;
			if (myImpl)
			{
				v1 = p * t * s * t2 * vertexList[faceList[i].x - 1];
				v2 = p * t * s * t2 * vertexList[faceList[i].y - 1];
				v3 = p * t * s * t2 * vertexList[faceList[i].z - 1];
			}
			else {
				v1 = t_z * s * t2 * vertexList[faceList[i].x - 1];
				v2 = t_z * s * t2 * vertexList[faceList[i].y - 1];
				v3 = t_z * s * t2 * vertexList[faceList[i].z - 1];
			}

			v1 /= v1.w;
			v2 /= v2.w;
			v3 /= v3.w;

			glm::vec3 N = -glm::normalize(getNormal(glm::vec3(v1), glm::vec3(v2), glm::vec3(v3)));
			vertexNormalArray[faceList[i].x - 1].push_back(N);
			vertexNormalArray[faceList[i].y - 1].push_back(N);
			vertexNormalArray[faceList[i].z - 1].push_back(N);
		}

		for (int i = 0; i < vertexList.size(); i++) {
			std::vector<glm::vec3> a = vertexNormalArray[i];
			
			if (a.size() == 0) {
				vertexNormals.push_back(glm::vec3(0,0,0));
				continue;
			}

			glm::vec3 n = glm::vec3(0, 0, 0);
			for (int j = 0; j < a.size(); j++) {
				n += a[j];
			}

			n /= a.size();

			vertexNormals.push_back(glm::normalize(n));
		}
	}

	for (int i = 0; i < faceList.size(); i++) {
		glm::vec4 v1;
		glm::vec4 v2;
		glm::vec4 v3;

		if (myImpl)
		{
			v1 =  p * t * s * t2 * vertexList[faceList[i].x - 1];
			v2 =  p * t * s * t2 * vertexList[faceList[i].y - 1];
			v3 =  p * t * s * t2 * vertexList[faceList[i].z - 1];
		}
		else {
			v1 = t_z * s * t2 * vertexList[faceList[i].x - 1];
			v2 = t_z * s * t2 * vertexList[faceList[i].y - 1];
			v3 = t_z * s * t2 * vertexList[faceList[i].z - 1];
		}

		v1 /= v1.w;
		v2 /= v2.w;
		v3 /= v3.w;

		glm::vec3 N = -glm::normalize(getNormal(glm::vec3(v1), glm::vec3(v2), glm::vec3(v3)));
		glm::vec3 normalP = getCenter(glm::vec3(v1), glm::vec3(v2), glm::vec3(v3)) - eye;
		glm::vec3 center = getCenter(glm::vec3(v1), glm::vec3(v2), glm::vec3(v3));

		double cosAlpha = glm::dot(N, normalP) / (glm::distance(N, glm::vec3(0, 0, 0)) * glm::distance(normalP, glm::vec3(0, 0, 0)));

		if (cosAlpha > 0) {
			continue;
		}

		glm::vec4 b1 = v2 - v1;
		glm::vec4 b2 = v3 - v2;
		glm::vec4 b3 = v1 - v3;	

		glm::vec3 L = glm::normalize(I - getCenter(glm::vec3(v1), glm::vec3(v2), glm::vec3(v3)));

		double LN = glm::dot(L, N);

		glm::vec3 intensity = glm::vec3(Ia.x * ka, Ia.y * ka, Ia.z * ka);			

		double cosAlphaDiffuse = glm::dot(N, L) / (glm::distance(N, glm::vec3(0, 0, 0)) * glm::distance(L, glm::vec3(0, 0, 0)));
		
		if (cosAlphaDiffuse > 0 && !gourad) {
			intensity += glm::vec3(color.x * kd * LN, color.y * kd * LN, color.z * kd * LN);
		}

		/*
		glBegin(GL_LINES);
		{
			glm::vec3 v = center + N;
			glm::vec3 v2 = center + L;
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(v.x, v.y, v.z);
			glVertex3f(center.x, center.y, center.z);
			glVertex3f(v2.x, v2.y, v2.z);
		}
		glEnd();
		glPushMatrix();
		GLUquadric *quad;
		quad = gluNewQuadric();
		glColor3ub(color.x, color.y, color.z);
		glTranslatef(center.x, center.y, center.z);
		gluSphere(quad, 0.05, 32, 32);
		glPopMatrix();*/
		
		float x1 = v1.x;
		float y1 = v1.y;
		float z1 = v1.z;

		float x2 = v2.x;
		float y2 = v2.y;
		float z2 = v2.z;

		float x3 = v3.x;
		float y3 = v3.y;
		float z3 = v3.z;
		/*
		glBegin(paint ? GL_TRIANGLES : GL_LINE_STRIP);
		{
			if (gourad) {
				glm::vec3 L1 = glm::normalize(I - glm::vec3(v1));
				glm::vec3 N1 = vertexNormals[faceList[i].x - 1];
				double LNV1 = glm::dot(L1, N1);

				glm::vec3 intensityV1 = intensity + 
					glm::vec3(color.x * kd * LNV1, color.y * kd * LNV1, color.z * kd * LNV1);

				double cos1 = glm::dot(N1, L1) / (glm::distance(N1, glm::vec3(0, 0, 0)) * glm::distance(L1, glm::vec3(0, 0, 0)));

				if (cos1 < 0) {
					intensityV1 = intensity;
				}

				glm::vec3 L2 = glm::normalize(I - glm::vec3(v2));
				glm::vec3 N2 = vertexNormals[faceList[i].y - 1];
				double LNV2 = glm::dot(L2, N2);

				glm::vec3 intensityV2 = intensity +
					glm::vec3(color.x * kd * LNV2, color.y * kd * LNV2, color.z * kd * LNV2);

				double cos2 = glm::dot(N2, L2) / (glm::distance(N2, glm::vec3(0, 0, 0)) * glm::distance(L2, glm::vec3(0, 0, 0)));

				if (cos2 < 0) {
					intensityV2 = intensity;
				}

				glm::vec3 L3 = glm::normalize(I - glm::vec3(v3));
				glm::vec3 N3 = vertexNormals[faceList[i].z - 1];
				double LNV3 = glm::dot(L3, N3);

				glm::vec3 intensityV3 = intensity +
					glm::vec3(color.x * kd * LNV3, color.y * kd * LNV3, color.z * kd * LNV3);

				double cos3 = glm::dot(N3, L3) / (glm::distance(N3, glm::vec3(0, 0, 0)) * glm::distance(L3, glm::vec3(0, 0, 0)));

				if (cos3 < 0) {
					intensityV3 = intensity;
				}

				glColor3ub(intensityV1.x, intensityV1.y, intensityV1.z);  glVertex3f(x1, y1, z1);
				glColor3ub(intensityV2.x, intensityV2.y, intensityV2.z);  glVertex3f(x2, y2, z2);
				glColor3ub(intensityV3.x, intensityV3.y, intensityV3.z);  glVertex3f(x3, y3, z3);
			}
			else {
				glColor3ub(intensity.x, intensity.y, intensity.z);  glVertex3f(x1, y1, z1);
				glColor3ub(intensity.x, intensity.y, intensity.z);  glVertex3f(x2, y2, z2);
				glColor3ub(intensity.x, intensity.y, intensity.z);  glVertex3f(x3, y3, z3);
			}
		}
		glEnd();*/

		glBegin(GL_LINE_STRIP);
		{
			for (int i2 = 1; i2 <= n - 3; i2++) {
				for (double t3 = 0; t3 <= 1; t3 += 0.1)
				{
					glm::vec4 p2 = p * t * s * t2 * segment(i2, t3);
					glVertex3f(p2.x, p2.y, p2.z);
				}
			}
		}
		glEnd();
	}

	glPushMatrix();
	GLUquadric *quad;
	quad = gluNewQuadric();
	glColor3ub(color.x, color.y, color.z);
	glTranslatef(I.x, I.y, I.z);
	//gluSphere(quad, 0.5, 32, 32);
	glPopMatrix();
}

void myMouse(int button, int state, int x, int y)
{
	//	Desna tipka - brise canvas. 
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		eye.x = 0;
		updatePerspective();
		glutPostRedisplay();
	}
}

void calculateMinimax()
{
	float minX, maxX, minY, maxY, minZ, maxZ;
	bool set = false;

	for (int i = 0; i < vertexList.size(); i++) {
		glm::vec4 v = vertexList[i];

		if (set)
		{
			minX = v.x < minX ? v.x : minX;
			maxX = v.x > maxX ? v.x : maxX;
			minY = v.y < minY ? v.y : minY;
			maxY = v.y > maxY ? v.y : maxY;
			minZ = v.z < minZ ? v.z : minZ;
			maxZ = v.z > maxZ ? v.z : maxZ;
		}
		else
		{
			set = true;
			minX = v.x;
			maxX = v.x;
			minY = v.y;
			maxY = v.y;
			minZ = v.z;
			maxZ = v.z;
		}
	}

	float size_x = maxX - minX;
	float size_y = maxY - minY;
	float size_z = maxZ - minZ;

	float center_x = (maxX + minX) / 2;
	float center_y = (maxY + minY) / 2;
	float center_z = (maxZ + minZ) / 2;

	float t2_values[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -center_x, -center_y, -center_z, 1 };
	t2 = glm::make_mat4(t2_values);

	float s_value;

	if (size_x > size_y)
	{
		s_value = size_z > size_x ? size_z : size_x;
	}
	else
	{
		s_value = size_z > size_y ? size_z : size_y;
	}

	s_value = 2 / s_value;

	float s_values[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 / s_value };
	s = glm::make_mat4(s_values);
}

void calculateTransformation()
{
	float t1_values[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -eye.x, -eye.y, -eye.z, 1 };
	t1 = glm::make_mat4(t1_values);

	H = glm::distance(eye, target);

	z = glm::normalize(eye - target);
	x = glm::normalize(glm::cross(z, viewUp));
	y = glm::normalize(glm::cross(x, z));

	float r_rot_values[9] = { x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z };
	glm::mat3 r_rot = glm::make_mat3(r_rot_values);

	float r_uku_values[16] = { x.x, y.x, z.x, 0, x.y, y.y, z.y, 0, x.z, y.z, z.z, 0, 0, 0, 0, 1 };
	glm::mat4 r_uku = glm::make_mat4(r_uku_values);

	float t_z_values[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
	t_z = glm::make_mat4(t_z_values);

	t = t_z * r_uku * t1;

	float p_values[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 / H, 0, 0, 0, 0 };
	p = glm::make_mat4(p_values);
}

void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
	switch (theKey)
	{
	case 'r': eye.x = 0.0;
		break;

	case 27:  exit(0);
		break;

	case 'w':
		eye.y = eye.y + 1;
		break;

	case 's':
		eye.y = eye.y - 1;
		break;

	case 'a':
		eye.x -= 1;
		break;

	case 'd':
		eye.x += 1;
		break;

	case 'q':
		eye.z += 1;
		break;

	case 'e':
		eye.z -= 1;
		break;

	case 'x':
		myImpl = !myImpl;
		if (myImpl) 
		{
			std::cout << "my implementation." << std::endl;
		}
		else
		{
			std::cout << "OpenGL function." << std::endl;
		}
		break;
	case 'i':
		I.y += 1;
		break;

	case 'k':
		I.y -= 1;
		break;

	case 'j':
		I.x -= 1;
		break;

	case 'l':
		I.x += 1;
		break;

	case 'u':
		I.z += 1;
		break;

	case 'o':
		I.z -= 1;
		break;
	case 'p':
		paint = !paint;
		break;
	case 'g':
		gourad = !gourad;
		break;
	}
	
	updatePerspective();
	glutPostRedisplay();
}

double baseFunction(int u, int k, int i) {
	if (k == 0) {
		if (u >= Uknot[i] && u < Uknot[i + 1]) {
			return 1;
		}
		else {
			return 0;
		}
	}

	double firstPart = u - Uknot[i];
	if (firstPart != 0) {
		firstPart /= Uknot[i + k] - Uknot[i];
		firstPart *= baseFunction(u, k - 1, i);
	}

	double secondPart = Uknot[i + k + 1] - u;
	if (secondPart != 0) {
		secondPart /= Uknot[i + k + 1] - Uknot[i + 1];
		secondPart *= baseFunction(u, k - 1, i + 1);
	}

	return firstPart + secondPart;
}

void calculateUknot() {
	for (int i = 0; i < k + 1; i++) {
		Uknot.push_back(0);
	}

	int m = n + k + 1;

	for (int i = 0; i < (m - 2 * k - 1); i++) {
		Uknot.push_back(i + 1);
	}

	int value = m - 2 * k;

	for (int i = 0; i < k + 1; i++) {
		Uknot.push_back(value);
	}
}

glm::vec4 segment(int i2, double t2) {
	float data[] = { t2*t2*t2, t2*t2, t2, 1 };
	glm::vec4 t3 = glm::make_vec4(data);
	
	float datar[] = { r[i2 - 1].x, r[i2 - 1].y, r[i2 - 1].z, 
		r[i2].x, r[i2].y, r[i2].z,
		r[i2 + 1].x, r[i2 + 1].y, r[i2 + 1].z,
		r[i2 + 2].x, r[i2 + 2].y, r[i2 + 2].z };

	glm::mat4 ri = glm::make_mat4(datar);

	glm::vec4 result = t3 * (Bi3 * ri) * (1.0f / 6);

	glm::to_string(result);

	return t3;
}