#include <stdlib.h>
#include <GL/glut.h>
#include <fstream>
#include <iostream>
#include <algorithm> 
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

#define PI 3.14
#define IX(x, y) ((x) + (y) * N)

int iter = 16;
int N = 128;
float SCALE = 4;
int velocityMultiplier = 100;
int candleVelocitiy = 1000;
int densityAmount = 40;
int velocityConstraint = 1000;
int mouseXlast = 0;
int mouseYlast = 0;
bool click = true;
int mode = 1; // 0 - candle, 1 - free-form
char color = 'w';
int flameSize = 10;
int candleWidth = 50;
int candleHeight = 100;

static bool checkIndex(int index) {
	if (index < 0 || index >= N * N) {
		return false;
	}

	return true;
}

static void set_bnd(int b, float *x)
{
	for (int i = 1; i < N - 1; i++) {
		x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
		x[IX(i, N - 1)] = b == 2 ? -x[IX(i, N - 2)] : x[IX(i, N - 2)];
	}

	for (int j = 1; j < N - 1; j++) {
		x[IX(0, j)] = b == 1 ? -x[IX(1, j)] : x[IX(1, j)];
		x[IX(N - 1, j)] = b == 1 ? -x[IX(N - 2, j)] : x[IX(N - 2, j)];
	}

	x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
	x[IX(0, N - 1)] = 0.5f * (x[IX(1, N - 1)] + x[IX(0, N - 2)]);
	x[IX(N - 1, 0)] = 0.5f * (x[IX(N - 2, 0)] + x[IX(N - 1, 1)]);
	x[IX(N - 1, N - 1)] = 0.5f * (x[IX(N - 2, N - 1)] + x[IX(N - 1, N - 2)]);
}

static void lin_solve(int b, float *x, float *x0, float a, float c)
{
	float cRecip = 1.0 / c;
	for (int k = 0; k < iter; k++) {
		for (int j = 1; j < N - 1; j++) {
			for (int i = 1; i < N - 1; i++) {
				x[IX(i, j)] =
					(x0[IX(i, j)]
						+ a * (x[IX(i + 1, j)]
							+ x[IX(i - 1, j)]
							+ x[IX(i, j + 1)]
							+ x[IX(i, j - 1)]
							)) * cRecip;
			}
		}
		set_bnd(b, x);
	}
}

static void diffuse(int b, float *x, float *x0, float diff, float dt)
{
	float a = dt * diff * (N - 2) * (N - 2);
	lin_solve(b, x, x0, a, 1 + 6 * a);
}

static void project(float *velocX, float *velocY, float *p, float *div)
{
	for (int j = 1; j < N - 1; j++) {
		for (int i = 1; i < N - 1; i++) {
			div[IX(i, j)] = -0.5f*(
				velocX[IX(i + 1, j)]
				- velocX[IX(i - 1, j)]
				+ velocY[IX(i, j + 1)]
				- velocY[IX(i, j - 1)]
				) / N;
			p[IX(i, j)] = 0;
		}
	}

	set_bnd(0, div);
	set_bnd(0, p);
	lin_solve(0, p, div, 1, 6);

	for (int j = 1; j < N - 1; j++) {
		for (int i = 1; i < N - 1; i++) {
			velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)]
				- p[IX(i - 1, j)]) * N;
			velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)]
				- p[IX(i, j - 1)]) * N;
		}
	}

	set_bnd(1, velocX);
	set_bnd(2, velocY);
}

static void advect(int b, float *d, float *d0, float *velocX, float *velocY, float dt)
{
	float i0, i1, j0, j1;

	float dtx = dt * (N - 2);
	float dty = dt * (N - 2);

	float s0, s1, t0, t1;
	float tmp1, tmp2, x, y;

	float Nfloat = N;
	float ifloat, jfloat;
	int i, j;

	for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++) {
		for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
			tmp1 = dtx * velocX[IX(i, j)];
			tmp2 = dty * velocY[IX(i, j)];
			x = ifloat - tmp1;
			x = ifloat - tmp1;
			y = jfloat - tmp2;

			if (x < 0.5f) x = 0.5f;
			if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
			i0 = floorf(x);
			i1 = i0 + 1.0f;
			if (y < 0.5f) y = 0.5f;
			if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
			j0 = floorf(y);
			j1 = j0 + 1.0f;

			s1 = x - i0;
			s0 = 1.0f - s1;
			t1 = y - j0;
			t0 = 1.0f - t1;

			int i0i = i0;
			int i1i = i1;
			int j0i = j0;
			int j1i = j1;

			int index1 = IX(i0i, j0i);
			int index2 = IX(i0i, j1i);
			int index3 = IX(i1i, j0i);
			int index4 = IX(i1i, j1i);

			if (!(checkIndex(index1)
				&& checkIndex(index2)
				&& checkIndex(index3)
				&& checkIndex(index4))) {
				return;
			}

			d[IX(i, j)] =
				s0 * (t0 * d0[index1] + t1 * d0[index2]) +
				s1 * (t0 * d0[index3] + t1 * d0[index4]);
		}
	}

	set_bnd(b, d);
}

static void drawRect(int x, int y, int w, int h) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();
}

static void drawCandle(int x, int y, int w, int h) {
	int wickHeight = h / 4;
	int wickWidth = 1;

	glColor3f(0.5f, 0.5f, 0.5f);
	drawRect(x - wickWidth / 2,
		y - wickHeight,
		wickWidth,
		wickHeight);

	glColor3f(0.5f, 0.01f, 0.01f);
	drawRect(x - flameSize / 2,
		y - flameSize / 2,
		flameSize,
		flameSize);

	glColor3f(0.5f, 0.35f, 0.05f);
	drawRect(x - w / 2,
		y - h,
		w - wickWidth,
		h - wickHeight);
}

class Fluid {
public:
	int size;
	float dt;
	float diff;
	float visc;

	float *s;
	float *density;

	float *Vx;
	float *Vy;

	float *Vx0;
	float *Vy0;

	Fluid(int diffusion, int viscosity, float dt) {
		int arraySize = N * N;

		this->size = N;
		this->dt = dt;
		this->diff = diffusion;
		this->visc = viscosity;

		this->s = (float *)calloc(arraySize, sizeof(float));
		this->density = (float *)calloc(arraySize, sizeof(float));

		this->Vx = (float *)calloc(arraySize, sizeof(float));
		this->Vy = (float *)calloc(arraySize, sizeof(float));

		this->Vx0 = (float *)calloc(arraySize, sizeof(float));
		this->Vy0 = (float *)calloc(arraySize, sizeof(float));
	}

	~Fluid() {
		free(this->s);
		free(this->density);

		free(this->Vx);
		free(this->Vy);

		free(this->Vx0);
		free(this->Vy0);
	}

	void addDensity(int x, int y, float amount) {
		int index = IX(x, y);
		if (!checkIndex(index)) {
			return;
		}
		this->density[index] += amount;
	}

	void addVelocity(int x, int y, float amountX, float amountY) {
		int index = IX(x, y);
		if (!checkIndex(index)) {
			return;
		}
		this->Vx[index] += amountX;
		this->Vy[index] += amountY;
	}

	void step() {
		float visc = this->visc;
		float diff = this->diff;
		float dt = this->dt;
		float *Vx = this->Vx;
		float *Vy = this->Vy;
		float *Vx0 = this->Vx0;
		float *Vy0 = this->Vy0;
		float *s = this->s;
		float *density = this->density;

		diffuse(1, Vx0, Vx, visc, dt);
		diffuse(2, Vy0, Vy, visc, dt);

		project(Vx0, Vy0, Vx, Vy);

		advect(1, Vx, Vx0, Vx0, Vy0, dt);
		advect(2, Vy, Vy0, Vx0, Vy0, dt);

		project(Vx, Vy, Vx0, Vy0);

		diffuse(0, s, density, diff, dt);
		advect(0, density, s, Vx, Vy, dt);
	}

	void render() {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				int x = i * SCALE;
				int y = j * SCALE;
				float d = this->density[IX(i, j)];

				switch (color) {
				case 'b':
					glColor3f(0, 0, d);
					break;
				case 'g':
					glColor3f(0, d, 0);
					break;
				case 'r':
					glColor3f(d, 0, 0);
					break;
				case 'w':
				default:
					glColor3f(d, d, d);
				}
				drawRect(x, y, SCALE, SCALE);
			}
		}
	}

	void fadeD() {
		for (int i = 0; i < N * N; i++) {
			this->density[i] /= 1.005;
		}
	}
};

GLuint window;
GLuint windowX = 100, windowY = 100;

int refreshMills = 10;

Fluid fluid = Fluid(0.2, 0, 0.0001);

void myReshape(int width, int height);
void OnDraw();
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void timer(int value);
void passiveMouse(int x, int y);
void onMouseButton(int button, int state, int x, int y);

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	glutInitWindowSize(N * SCALE, N * SCALE);
	glutInitWindowPosition(windowX, windowY);

	window = glutCreateWindow("Fluid simulation");

	glutDisplayFunc(OnDraw);
	glutReshapeFunc(myReshape);
	glutKeyboardFunc(myKeyboard);
	glutTimerFunc(0, timer, 0);
	glutMotionFunc(passiveMouse);
	glutMouseFunc(onMouseButton);

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
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OnDraw()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	fluid.step();
	fluid.render();
	fluid.fadeD();

	if (mode == 0) {
		fluid.addDensity(mouseXlast / SCALE, N - mouseYlast / SCALE, rand() % densityAmount / 2);

		for (int i = 0; i < 2; i++) {
			float angle = rand() % 100;
			angle /= 100;
			angle *= PI;

			fluid.addVelocity(mouseXlast / SCALE, N - mouseYlast / SCALE, candleVelocitiy * cos(angle), candleVelocitiy * sin(angle));
		}

		drawCandle(mouseXlast, N * SCALE - mouseYlast, candleWidth, candleHeight);
	}

	glutSwapBuffers();
}


void myKeyboard(unsigned char theKey, int mouseX, int mouseY)
{
	switch (theKey) {
	case 'm':
		mode = 1 - mode;
		break;
	case 'b':
		color = 'b';
		break;
	case 'r':
		color = 'r';
		break;
	case 'g':
		color = 'g';
		break;
	case 'w':
		color = 'w';
		break;
	}

	glutPostRedisplay();
}

void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(refreshMills, timer, 0);
}

void passiveMouse(int x, int y)
{
	if (click) {
		mouseXlast = x;
		mouseYlast = y;
		click = false;
		return;
	}

	float amountX = std::max(std::min(velocityMultiplier * (x - mouseXlast), velocityConstraint), -velocityConstraint);
	float amountY = std::max(std::min(velocityMultiplier * (mouseYlast - y), velocityConstraint), -velocityConstraint);

	if (mode == 1) {
		fluid.addDensity(x / SCALE, N - y / SCALE, densityAmount);
		fluid.addVelocity(x / SCALE, N - y / SCALE, amountX, amountY);
	}
	mouseXlast = x;
	mouseYlast = y;
	glutPostRedisplay();
}

void onMouseButton(int button, int state, int x, int y)
{
	click = true;

	mouseXlast = x;
	mouseYlast = y;
}
