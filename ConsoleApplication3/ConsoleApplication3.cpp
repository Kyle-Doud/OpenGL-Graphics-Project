/*
*	INSTRUCTIONS FOR USE
*	Use the arrow keys to rotate the camera around the character.
*   The up arrow key will zoom in on the student, while the down arrow key will zoom out
*	so you can see more of the world at once
*	Use W A S D to rotate and move character forward/back
*	x makes the "student" grow, z makes her shrink
*	The student starts in front of the Shelby center with 2 friends.
*	C switches between the characters
*	T initiates or cancels a tour
*	F toggles the tour flashlight
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>
#include "ConsoleApplication3.h"

#define RIGHT true
#define LEFT false

#define FORWARD true
#define BACK false

#define INCREASE true
#define DECREASE false

#define M_PI 3.141592653589793238462

typedef float point[4];

// angle of rotation for the camera direction
float angle = 270.0;
float angley = 30.0;
// actual vector representing the camera's direction
float lx = 0.0f, ly = 0.0f, lz = 0.0f;
// XZ position of the camera
float x = 0.0f, y = 800.0f, z = 0.0f;

float tourXYZ[3] = { 12000.0f, 400.0f, -14000.0f };
float tourLookXYZ[3] = { -1.0f, 0.0f, 0.0f };
float tourCameraAngle = 0;

double theta[] = { 0, 0, 0 };
float currentBodyCenterPositionXZ[3][2] = { { 10000.0, -12000.0 }, { 11000, -13000 }, { 12000, -12000 } };
double leftArmRotation = 0;
double rightArmRotation = 0;
bool limbRoatationPositive = true;
double gscale = 1;
int curStudent = 0;
int boxCount = 0;
int tourPhase = -1;
bool switchedOn = false;
GLboolean tour = false;
GLboolean initBoundaries = true;

struct BoundaryBox {
	float upperLeftX;
	float upperLeftY;
	float xLength;
	float yLength;
};

BoundaryBox studentBoxes[3];
BoundaryBox buildingBoxes[100];
BoundaryBox camBox;

GLfloat vertices[][3] = { { -1.0,-1.0,-1.0 },{ 1.0,-1.0,-1.0 },
{ 1.0,1.0,-1.0 },{ -1.0,1.0,-1.0 },{ -1.0,-1.0,1.0 },
{ 1.0,-1.0,1.0 },{ 1.0,1.0,1.0 },{ -1.0,1.0,1.0 }, 
{ 0.0, -1.0, -1.0 },
{ 1.0, 1.0, -1.0 }, 
{ -1.0, 1.0, -1.0 },
{ 0.0, -1.0, 1.0 }, 
{ 1.0, 1.0, 1.0 }, 
{ -1.0, 1.0, 1.0 }, };

GLfloat normals[][3] = { { -1.0,-1.0,-1.0 },{ 1.0,-1.0,-1.0 },
{ 1.0,1.0,-1.0 },{ -1.0,1.0,-1.0 },{ -1.0,-1.0,1.0 },
{ 1.0,-1.0,1.0 },{ 1.0,1.0,1.0 },{ -1.0,1.0,1.0 },
{ 0.0, -1.0, -1.0 },//index 8
{ 1.0, 1.0, -1.0 },
{ -1.0, 1.0, -1.0 },
{ 0.0, -1.0, 1.0 },
{ 1.0, 1.0, 1.0 },
{ -1.0, 1.0, 1.0 }, };

GLfloat light_ambient[] = { 0.2, 0.2, 0.2, 1 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat sunlight_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat sunlight_specular[] = { 1.0, 1.0, 1.0, 1.0 };

int lightMode = 0;
#define LIGHTMODE_AMBIENT 3
#define LIGHTMODE_SPOTLIGHT 0
#define LIGHTMODE_SUN 1
#define LIGHTMODE_BOTH 2
GLfloat lightPosition[4];
GLfloat lightDirection[3];

void myinit()
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess = { 1.0 };
	GLfloat ambientLight[] = { 0.2, 0.2, 0.2, 1.0 };


	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	glEnable(GL_LIGHTING); /* enable lighting */
	
	glColorMaterial(GL_FRONT, GL_AMBIENT);
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(.4f, .8f, .5f, 0.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	y += 100 * tan(angley * (3.1415926535 / 180));

	glMatrixMode(GL_MODELVIEW);              // setup viewing projection 
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_SMOOTH);   // Enable smooth shading

}

GLboolean isCollision(BoundaryBox one, BoundaryBox two)
{
	bool xCollision = one.upperLeftX + one.xLength >= two.upperLeftX &&
		two.upperLeftX + two.xLength >= one.upperLeftX;
	bool yCollision = one.upperLeftY <= two.upperLeftY + two.yLength &&
		two.upperLeftY <= one.upperLeftY + one.yLength;
	return (xCollision && yCollision);
}

GLboolean checkForCollisions()
{
	bool collisionDetected = false;
	for (int i = 0; i < 4; i++)
	{
		if ((curStudent != i) && !collisionDetected) //don't check for collisions between an object and itself
		{
			collisionDetected = isCollision(studentBoxes[curStudent], studentBoxes[i]);
		}
	}
	for (int build = 0; build < 100; build++)
	{
		if (!collisionDetected)
		{
			collisionDetected = isCollision(studentBoxes[curStudent], buildingBoxes[build]);
		}
	}
	return collisionDetected;
}

GLboolean checkForCamCollisions()
{
	bool collisionDetected = false;
	for (int i = 0; i < 4; i++)
	{
		if (!collisionDetected)
		{
			collisionDetected = isCollision(camBox, studentBoxes[i]);
		}
	}
	for (int build = 0; build < 100; build++)
	{
		if (!collisionDetected)
		{
			collisionDetected = isCollision(camBox, buildingBoxes[build]);
		}
	}
	return collisionDetected;
}

void normal(point p)
{
	/* normalize a vector */

	float sqrt();
	float d = 0.0;
	int i;
	for (i = 0; i<3; i++) d += p[i] * p[i];
	d = sqrtf(d);
	if (d>0.0)
		for (i = 0; i<3; i++) p[i] /= d;
}

void copyPt(const point pIn, point pOut)
{
	pOut[0] = pIn[0];
	pOut[1] = pIn[1];
	pOut[2] = pIn[2];
}

void rectangle(const GLfloat *a, const GLfloat *b, const GLfloat *c, const GLfloat *d)
{
	/* draw a polygon via list of vertices */

	glBegin(GL_POLYGON);
	point normalize;
	copyPt(a, normalize);
	normal(normalize);
	glNormal3fv(normalize);
	glVertex3fv(a);

	copyPt(b, normalize);
	normal(normalize);
	glNormal3fv(normalize);
	glVertex3fv(b);

	copyPt(c, normalize);
	normal(normalize);
	glNormal3fv(normalize);
	glVertex3fv(c);

	copyPt(d, normalize);
	normal(normalize);
	glNormal3fv(normalize);
	glVertex3fv(d);
	glEnd();
}

void divideRectangle(const GLfloat *a, const GLfloat *b, const GLfloat *c, const GLfloat *d, int m)
{
	point v1, v2, v3, v4, v5;
	int j;
	if (m>0)
	{
		for (j = 0; j<3; j++) v1[j] = (a[j] + b[j]) / 2.0;
		for (j = 0; j<3; j++) v2[j] = (a[j] + c[j]) / 2.0;
		for (j = 0; j<3; j++) v3[j] = (b[j] + c[j]) / 2.0;
		for (j = 0; j<3; j++) v4[j] = (c[j] + d[j]) / 2.0;
		for (j = 0; j<3; j++) v5[j] = (a[j] + d[j]) / 2.0;
		divideRectangle(v1, v2, v3, b, m - 1);
		divideRectangle(v2, v4, c, v3, m - 1);
		divideRectangle(a, v5, v2, v1, m - 1);
		divideRectangle(v5, d, v4, v2, m - 1);
	}
	else
	{
		//glColor3b(rand() % 255, rand() % 255, rand() % 255);
		rectangle(a, b, c, d); /* draw rectangle at end of recursion */
	}
}

void preDivideRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	GLfloat v1[] = { x1, y1, 0 };
	GLfloat v2[] = { x2, y1, 0 };
	GLfloat v3[] = { x2, y2, 0 };
	GLfloat v4[] = { x1, y2, 0 };

	divideRectangle(v1, v2, v3, v4, 2);
}

void defineBoundaries(GLfloat xLengthIn, GLfloat yLengthIn, GLfloat upperLeftXIn, GLfloat upperLeftYIn)
{
	buildingBoxes[boxCount].xLength = xLengthIn * 27;
	buildingBoxes[boxCount].yLength = -(yLengthIn * 27);
	buildingBoxes[boxCount].upperLeftX = upperLeftXIn * 27;
	buildingBoxes[boxCount].upperLeftY = -(upperLeftYIn * 27) - 2900;
	boxCount++;
}

//@param x1 = bottom left, y1 = bottom left, x2 = top right, y2 = top right
void rectToCube(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	GLfloat v1[] = { x1, y1, 0 };//bottom left
	GLfloat v2[] = { x2, y1, 0 };//bottom right
	GLfloat v3[] = { x2, y2, 0 };//top right
	GLfloat v4[] = { x1, y2, 0 };//top left

	GLfloat v5[] = { x1, y1, 100 };
	GLfloat v6[] = { x2, y1, 100 };
	GLfloat v7[] = { x2, y2, 100 };
	GLfloat v8[] = { x1, y2, 100 };

	if(initBoundaries)
	defineBoundaries(x2 - x1, y2 - y1, x1, y2);

	divideRectangle(v1, v4, v3, v2, 2);
	divideRectangle(v3, v4, v8, v7, 2);
	divideRectangle(v1, v5, v8, v4, 2);
	divideRectangle(v2, v3, v7, v6, 2);
	divideRectangle(v5, v6, v7, v8, 2);
	divideRectangle(v1, v2, v6, v5, 2);

}



void createDavis()
{
	rectToCube(2140, 565, 2235, 500);//bottom
	rectToCube(2150, 555, 2275, 285);//the rest
}

void createHarbertCenter()
{
	rectToCube(1945, 260, 2245, 150);
}

void createRoss()
{
	rectToCube(1770, 820, 2005, 700);//main
	rectToCube(1790, 805, 1990, 685);//outer
	rectToCube(1850, 885, 1920, 808);//bottom
}

void createWilmoreLaboratories()
{
	rectToCube(1555, 630, 1785, 355);//inner
	rectToCube(1505, 605, 1840, 375);//outer
}

void createRamsayHall()
{
	rectToCube(1520, 275, 1565, 170);//left
	rectToCube(1520, 250, 1830, 170);//middle
	rectToCube(1780, 275, 1830, 170);//right
}

void createTextileEngineering()
{
	rectToCube(1005, 255, 1405, 175);//main rect
	rectToCube(1185, 280, 1220, 175);//middle
	rectToCube(990, 230, 1024, 190);//thing on left
	rectToCube(1025, 225, 1065, 170);//other thing on left
	rectToCube(1340, 255, 1385, 170);//thing on right
}

void createLBuilding()
{
	rectToCube(1010, 760, 1040, 680);//bottom left
	rectToCube(1010, 740, 1405, 680);//bottom long
	rectToCube(1340, 745, 1405, 475);//right long
	rectToCube(1320, 560, 1405, 475);//right top
}

void createEngineeringShops()
{
	rectToCube(1070, 615, 1260, 535);//bottom
	rectToCube(1070, 500, 1260, 430);//middle
	rectToCube(1070, 395, 1260, 315);//top
	rectToCube(1230, 615, 1260, 315);//side
}

void createBroun()
{
	rectToCube(585, 965, 758, 832);//bottom left corner to ??
	rectToCube(620, 850, 850, 790);//next from bottom
	rectToCube(630, 790, 850, 750);//top
	rectToCube(720, 985, 830, 750);//bottommost weird thing
	rectToCube(830, 895, 865, 850);//weirdness on the right
}

void createAERL()
{
	rectToCube(575, 710, 705, 570);//Middle
	rectToCube(510, 675, 765, 570);//top
	rectToCube(515, 685, 555, 555);//left fancy
	rectToCube(260, 685, 305, 555);//right fancy
}

void createWiggins()
{
	rectToCube(120, 745, 245, 570);//Middle
	rectToCube(55, 675, 310, 570);//top
	rectToCube(60, 685, 105, 555);//left fancy
	rectToCube(715, 685, 755, 555);//right fancy
}

void createShelby()
{
	//West Side
	glPushMatrix();
	glColor3f(1, 1, 1);
	glutSolidSphere(10, 10, 10);
	glPopMatrix();
	glColor3f(0.2, 0.2, 0.8);
	rectToCube(60, 210, 135, 485);
	rectToCube(55, 420, 160, 460);
	rectToCube(60, 210, 260, 285);
	rectToCube(260, 320, 305, 205);

	rectToCube(260.0, 315.0, 310.0, 210.0);
	rectToCube(55.0, 310.0, 105.0, 205.0);
	//End of West side

	//Middle
	rectToCube(335, 350, 485, 265);//innermost rect
	rectToCube(345, 370, 470, 240);
	rectToCube(365, 380, 450, 225);//outermost rect
								//End of Middle

								//East Side
	rectToCube(655, 460, 765, 420);//bottommost rect
	rectToCube(685, 460, 755, 210);//big one on the right
	rectToCube(715, 315, 765, 205);//top right
	rectToCube(510, 285, 730, 210);//big one on top
	rectToCube(515, 205, 560, 320);//extra one on left
	rectToCube(507, 315, 560, 210);//fancy extra one on left
								//End of East Side

}

void createBuildings()
{
	glColor3f(0.2, 0.2, 0.8);
	glShadeModel(GL_SMOOTH);
	createShelby();
	createWiggins();
	createAERL();
	createBroun();
	createEngineeringShops();
	createLBuilding();
	createTextileEngineering();
	createRamsayHall();
	createWilmoreLaboratories();
	createRoss();
	createHarbertCenter();
	createDavis();
	initBoundaries = false;
}

void createMagnolia()
{
	preDivideRect(0, 100, 2400, 35);
	preDivideRect(590, 35, 640, 0);
	preDivideRect(1300, 35, 1350, 0);
	preDivideRect(2200, 35, 2250, 0);
}

void createFoyLoop()
{
	preDivideRect(1430, 800, 1460, 50);
	preDivideRect(975, 800, 1460, 820);
	preDivideRect(975, 990, 1260, 820);//parking lot
	preDivideRect(970, 990, 1000, 635);//up
	preDivideRect(970, 660, 1300, 635);//right
	preDivideRect(1030, 660, 1055, 280);//left "up"
	preDivideRect(1275, 660, 1300, 280);//right "up"
	preDivideRect(1030, 305, 1275, 280);//left top
	preDivideRect(1275, 465, 1455, 440);//right middle
}

void createMainRoads()
{
	glColor3f(.574, .543, .496);
	glShadeModel(GL_SMOOTH);
	createMagnolia();
	createFoyLoop();
}

void createTrees()
{
	glColor3f(.0, .9, .3);
	glShadeModel(GL_SMOOTH);
	rectToCube(950, 325, 960, 320);
	rectToCube(950, 345, 960, 340);
	rectToCube(950, 365, 960, 360);
	rectToCube(950, 385, 960, 380);
	rectToCube(950, 405, 960, 400);
	rectToCube(950, 425, 960, 420);
	rectToCube(950, 445, 960, 440);
	rectToCube(950, 465, 960, 460);
	rectToCube(950, 485, 960, 480);
	rectToCube(950, 505, 960, 500);
	//end of west row out my lab window

	rectToCube(1000, 525, 1010, 520);
	rectToCube(1010, 535, 1020, 530);
	rectToCube(1000, 545, 1010, 540);
	rectToCube(1010, 555, 1020, 550);
	rectToCube(1000, 565, 1010, 560);
	rectToCube(1010, 575, 1020, 570);
	rectToCube(1010, 595, 1020, 590);
	rectToCube(1010, 615, 1020, 610);
	rectToCube(1010, 625, 1020, 620);
	//end of east row out my lab window

}

void createMulch()
{
	glColor3f(0.47, 0.3, 0.3);
	glShadeModel(GL_SMOOTH);
	preDivideRect(935, 510, 980, 305);
	preDivideRect(990, 635, 1030, 510);
}

void createWalkingPaths()
{
	glColor3f(1.0, 1.0, 0.5);
	glShadeModel(GL_SMOOTH);
	preDivideRect(390, 225, 430, 100);//from magnolia
	preDivideRect(315, 210, 500, 180);//T
	preDivideRect(315, 645, 325, 180);//left of T
	preDivideRect(490, 645, 500, 180);//right of T
	preDivideRect(315, 530, 500, 375);//big space beneath shelby
	preDivideRect(315, 645, 375, 530);//left below big space
	preDivideRect(440, 645, 500, 530);//right below big space
	preDivideRect(315, 645, 500, 605);//bottom crossbar

	preDivideRect(0, 530, 805, 515);//lowder-to-concourse lower
	preDivideRect(0, 485, 805, 470);//lowder-to-concourse upper

	preDivideRect(790, 545, 875, 530);//stairs lower
	preDivideRect(790, 470, 875, 455);//stairs upper

	preDivideRect(870, 895, 890, 100);//concourse

	preDivideRect(1460, 330, 2115, 315);//wilmore to davis

	preDivideRect(1460, 670, 2115, 655);//wilmore to davis lower

	preDivideRect(2070, 615, 2140, 260);//davis patio

	preDivideRect(2140, 500, 2150, 260);//davis patio
}

void drawWaterfall()
{
	glPushMatrix();
	glColor3f(0.5f, 0.0f, 1.0f);
	glShadeModel(GL_SMOOTH);
	glTranslatef(835, 500, 0);
	glScalef(1.5, 1.5, 1);
	rectToCube(-25, -25, 25, 25);
	glPopMatrix();
}

void drawLeftShelbyCourtyard()
{
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glTranslatef(605, 365, 0);
	glScalef(1.5, 1.5, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawRightShelbyCourtyard()
{
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glTranslatef(215, 365, 0);
	glScalef(1.5, 1.5, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}

void drawLeftShelbyCourtyard2()
{
	glPushMatrix();
	glColor3f(0.47, 0.3, 0.3);
	glShadeModel(GL_SMOOTH);
	glTranslatef(530, 365, 0);
	glScalef(1.0, 1.5, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawRightShelbyCourtyard2()
{
	glPushMatrix();
	glColor3f(0.47, 0.3, 0.3);
	glShadeModel(GL_SMOOTH);
	glTranslatef(290, 365, 0);
	glScalef(1.0, 1.5, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}

void drawLeftShelbyCourtyard3()
{
	glPushMatrix();
	glColor3f(0.47, 0.3, 0.3);
	glShadeModel(GL_SMOOTH);
	glTranslatef(575, 440, 0);
	glScalef(2.5, 0.7, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawRightShelbyCourtyard3()
{
	glPushMatrix();
	glColor3f(0.47, 0.3, 0.3);
	glShadeModel(GL_SMOOTH);
	glTranslatef(240, 440, 0);
	glScalef(2.5, 0.7, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}

void drawLeftShelbyCourtyardEdge1()
{
	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(680, 350, 0);
	glScalef(1.6, 2.8, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawLeftShelbyCourtyardEdge2()
{
	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(635, 302, 0);
	glScalef(3.0, 1.0, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawRightShelbyCourtyardEdge1()
{
	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(140, 350, 0);
	glScalef(1.6, 2.8, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}
void drawRightShelbyCourtyardEdge2()
{
	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(185, 302, 0);
	glScalef(3.0, 1.0, 0);
	preDivideRect(-25, -25, 25, 25);
	glPopMatrix();
}

void drawTree()
{
	glColor3f(0.0, .8, 0.0);
	glShadeModel(GL_SMOOTH);
	glBegin(GL_POLYGON);
	float x, y;
	for (int angle = 0; angle < 360; angle++)
	{
		x = 10.0 * cos(angle);
		y = 10.0 * sin(angle);
		glVertex2f(x, y);
	}
	glEnd();
}

void drawTree1()
{
	glPushMatrix();
	glTranslatef(200, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree2()
{
	glPushMatrix();
	glTranslatef(215, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree3()
{
	glPushMatrix();
	glTranslatef(230, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree4()
{
	glPushMatrix();
	glTranslatef(245, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree5()
{
	glPushMatrix();
	glTranslatef(260, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree6()
{
	glPushMatrix();
	glTranslatef(560, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree7()
{
	glPushMatrix();
	glTranslatef(575, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree8()
{
	glPushMatrix();
	glTranslatef(590, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree9()
{
	glPushMatrix();
	glTranslatef(605, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}
void drawTree10()
{
	glPushMatrix();
	glTranslatef(620, 440, 0);
	glScalef(.5, .5, 0);
	drawTree();
	glPopMatrix();
}

void drawTransformationObjects()
{
	glPushMatrix();
	glScalef(1, 1, 2);
	drawWaterfall();//1
	glPopMatrix();
	drawLeftShelbyCourtyard();//2
	drawRightShelbyCourtyard();//3
	drawLeftShelbyCourtyard2();//4
	drawRightShelbyCourtyard2();//5
	drawLeftShelbyCourtyard3();//6
	drawRightShelbyCourtyard3();//7
	drawLeftShelbyCourtyardEdge1();//8
	drawLeftShelbyCourtyardEdge2();//9
	drawRightShelbyCourtyardEdge1();//10
	drawRightShelbyCourtyardEdge2();//11
	drawTree1();//12
	drawTree2();//13
	drawTree3();//14
	drawTree4();//15
	drawTree5();//16
	drawTree6();//17
	drawTree7();//18
	drawTree8();//19
	drawTree9();//20
	drawTree10();
}

void draw_Static_Objects()
{
	glPushMatrix();
	glScalef(27, 15, 27);
	glRotatef(-90, 1, 0, 0);
	
	drawTransformationObjects();

	createBuildings();

	createMainRoads();

	createMulch();

	createTrees();

	createWalkingPaths();
	glPopMatrix();
}

void polygon(int a, int b, int c, int d)
{
	/* draw a polygon via list of vertices */

	glBegin(GL_POLYGON);
	glNormal3fv(normals[a]);
	glVertex3fv(vertices[a]);

	glNormal3fv(normals[b]);
	glVertex3fv(vertices[b]);

	glNormal3fv(normals[c]);
	glVertex3fv(vertices[c]);

	glNormal3fv(normals[d]);
	glVertex3fv(vertices[d]);
	glEnd();
}

void triangle(int a, int b, int c)
{
	glBegin(GL_POLYGON);
	glNormal3fv(normals[a]);
	glVertex3fv(vertices[a]);

	glNormal3fv(normals[b]);
	glVertex3fv(vertices[b]);

	glNormal3fv(normals[c]);
	glVertex3fv(vertices[c]);

	glEnd();
}

void drawCube()
{
	polygon(0, 3, 2, 1);
	polygon(2, 3, 7, 6);
	polygon(0, 4, 7, 3);
	polygon(1, 2, 6, 5);
	polygon(4, 5, 6, 7);
	polygon(0, 1, 5, 4);
}

void drawWedge()
{
	triangle(8, 9, 10);
	triangle(11, 12, 13);
	polygon(8, 11, 12, 9);
	polygon(9, 12, 13, 10);
	polygon(8, 10, 13, 11);
}

void createHead()
{
	glPushMatrix();
	glTranslatef(0, 300, 0);

	glColor3f(0.0, 0.0, 0.0);//hair
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0, 10.0, 0.0);
	glScalef(90, 70, 70);
	drawCube();
	glPopMatrix();

	glPushMatrix();//face
	glColor3f(1.17, 0.21, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, -30.0, 60.0);
	glScalef(30, 30, 20);
	drawCube();
	glPopMatrix();

	glPushMatrix();//neck
	glColor3f(1.17, 0.21, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, -70.0, 0.0);
	glScalef(10, 15, 10);
	drawCube();
	glPopMatrix();

	glPopMatrix();
	
}

void createGlasses()
{
	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(15.0, 283.0, 70.0);
	glScalef(15, 15, 15);
	drawWedge();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.8, 0.8, 0.8);
	glShadeModel(GL_SMOOTH);
	glTranslatef(-15.0, 283.0, 70.0);
	glScalef(15, 15, 15);
	drawWedge();
	glPopMatrix();
}

void createBackpack()
{
	glColor3f(0.945, 0.92, 0.495);
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -70.0);
	glScalef(50, 50, 30);
	drawCube();
	glPopMatrix();
}

void createBody(int studentNum)
{
	glPushMatrix();
	glTranslatef(0, 180, 0);

	glColor3f(1.0, 0.7, 1.0);//chest
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(40, 40, 40);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//waist
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, -40.0, 0.0);
	glScalef(15, 35, 15);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//groin
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, -100.0, -20.0);
	glScalef(35, 30, 50);
	drawWedge();
	glPopMatrix();

	if (studentNum == curStudent)
	{
		createBackpack();
	}
	glPopMatrix();
}
void createLeftArm(int studentNum)
{
	glPushMatrix();
	
	glTranslatef(60, 180, 0);
	if (studentNum == curStudent)
	{
		glRotatef(leftArmRotation, 1, 0, 0);
	}
	glPushMatrix();
	glColor3f(0.96, 0.0, 0.22);//shoulder
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(25, 25, 25);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//arm
	glShadeModel(GL_SMOOTH);
	glRotatef(35, 0, 0, 1);
	glTranslatef(0.0, -80.0, 0.0);
	glScalef(10, 80, 10);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.17, 0.21, 0.8);//hand
	glShadeModel(GL_SMOOTH);
	glRotatef(35, 0, 0, 1);
	glTranslatef(0.0, -170.0, 0.0);
	glScalef(10, 10, 10);
	drawCube();
	glPopMatrix();

	glPopMatrix();
}

void createBook()
{
	glPushMatrix();
	glColor3f(0.0, 1.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glRotatef(-25, 0, 0, 1);
	glTranslatef(-10.0, -190.0, 0.0);
	glScalef(10, 30, 50);
	drawCube();
	glPopMatrix();
}

void createRightArm(int studentNum)
{
	glPushMatrix();
	
	glTranslatef(-60, 180, 0);
	if (studentNum == curStudent)
	{
		glRotatef(rightArmRotation, 1, 0, 0);
	}

	glPushMatrix();
	glColor3f(2.17, 0.43, 1.5);//shoulder
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(25, 25, 25);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//arm
	glShadeModel(GL_SMOOTH);
	glRotatef(-35, 0, 0, 1);
	glTranslatef(0.0, -80.0, 0.0);
	glScalef(10, 80, 10);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.17, 0.21, 0.8);//hand
	glShadeModel(GL_SMOOTH);
	glRotatef(-35, 0, 0, 1);
	glTranslatef(0.0, -170.0, 0.0);
	glScalef(10, 10, 10);
	drawCube();
	glPopMatrix();

	if (studentNum == curStudent)
	{
		createBook();
	}

	glPopMatrix();
}



void createLeftLeg(int studentNum)
{
	glPushMatrix();
	glTranslatef(50, 30, -20);
	if (studentNum == curStudent)
	{
	glRotatef(rightArmRotation, 1, 0, 0);
}

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//thigh
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(40, 70, 50);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//calf
	glShadeModel(GL_SMOOTH);
	glTranslatef(-10.0, -100.0, 0.0);
	glScalef(20, 80, 20);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);//foot
	glShadeModel(GL_SMOOTH);
	glTranslatef(-10.0, -200.0, 10.0);
	glScalef(20, 20, 30);
	drawCube();
	glPopMatrix();

	glPopMatrix();

}
void createRightLeg(int studentNum)
{
	glPushMatrix();
	glTranslatef(-50, 30, -20);
	if (studentNum == curStudent)
	{
		glRotatef(leftArmRotation, 1, 0, 0);
	}

	glPushMatrix();
	glColor3f(0.96, 0.0, 0.22);//thigh
	glShadeModel(GL_SMOOTH);
	glTranslatef(0.0, 0.0, 0.0);
	glScalef(40, 70, 45);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.96, 0.0, 0.22);//calf
	glShadeModel(GL_SMOOTH);
	glTranslatef(10.0, -100.0, 0.0);
	glScalef(20, 80, 20);
	drawCube();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.96, 0.0, 0.22);//foot
	glShadeModel(GL_SMOOTH);
	glTranslatef(10.0, -200.0, 10.0);
	glScalef(20, 20, 30);
	drawCube();
	glPopMatrix();

	glPopMatrix();
}



void createStudent()
{
	createHead();
	createBody(0);
	createLeftArm(0);
	createRightArm(0);
	createLeftLeg(0);
	createRightLeg(0);
	createGlasses();
}

void createOther(int stuID)
{
	createHead();
	createBody(stuID);
	createLeftArm(stuID);
	createRightArm(stuID);
	createLeftLeg(stuID);
	createRightLeg(stuID);
}

void placeOthers()
{
	glPushMatrix();
	glTranslatef(currentBodyCenterPositionXZ[1][0], 300, currentBodyCenterPositionXZ[1][1]);

		glRotatef(theta[1], 0, 1, 0);

	glScalef(gscale, gscale, gscale);
	createOther(1);
	studentBoxes[1].xLength = 150;
	studentBoxes[1].yLength = 150;
	studentBoxes[1].upperLeftX = (currentBodyCenterPositionXZ[1][0] - (studentBoxes[1].xLength / 2));
	studentBoxes[1].upperLeftY = (currentBodyCenterPositionXZ[1][1] + (studentBoxes[1].yLength / 2));
	glPopMatrix();

	glPushMatrix();
	glTranslatef(currentBodyCenterPositionXZ[2][0], 300, currentBodyCenterPositionXZ[2][1]);

		glRotatef(theta[2], 0, 1, 0);

	glScalef(gscale, gscale, gscale);
	createOther(2);
	studentBoxes[2].xLength = 150;
	studentBoxes[2].yLength = 150;
	studentBoxes[2].upperLeftX = (currentBodyCenterPositionXZ[2][0] - (studentBoxes[2].xLength / 2));
	studentBoxes[2].upperLeftY = (currentBodyCenterPositionXZ[2][1] + (studentBoxes[2].yLength / 2));
	glPopMatrix();
}

void placeStudent()
{
	glPushMatrix();
	glTranslatef(currentBodyCenterPositionXZ[0][0], 300, currentBodyCenterPositionXZ[0][1]);
		glRotatef(theta[0], 0, 1, 0);
	glScalef(gscale, gscale, gscale);
	createStudent();
	studentBoxes[0].xLength = 100;
	studentBoxes[0].yLength = 100;
	studentBoxes[0].upperLeftX = (currentBodyCenterPositionXZ[0][0] - (studentBoxes[0].xLength / 2));
	studentBoxes[0].upperLeftY = (currentBodyCenterPositionXZ[0][1] + (studentBoxes[0].yLength / 2));
	glPopMatrix();
}

float cosd(float degrees)
{
	float angleradians = degrees * (M_PI / 180.0f);
	return cos(angleradians);
}

float sind(float degrees)
{
	float angleradians = degrees * (M_PI / 180.0f);
	return sin(angleradians);
}

void doTourMovement()
{
	switch (tourPhase)
	{
	case -1:
		if (tourCameraAngle < 360)
		{
			tourLookXYZ[0] +=  200 * cosd(tourCameraAngle);
			tourLookXYZ[2] +=  200 * sind(tourCameraAngle);
			tourCameraAngle += 0.2f;
		}
		else
		{
			tourPhase = 0;
			tourCameraAngle = 0;
		}
		break;
	case 0:
		if (tourXYZ[0] < 20500)
		{
			tourXYZ[0] += 10.0f;
			tourLookXYZ[0] += 10.0f;
		}
		else
		{
			tourPhase = 1;
		}
		break;
	case 1:
		if (tourCameraAngle < 90)
		{
			tourCameraAngle += 1.0f;
			tourLookXYZ[0] -= cosd(tourCameraAngle);
			tourLookXYZ[2] += sind(tourCameraAngle);
		}
		else
		{
			tourPhase = 2;
		}
		break;
	case 2:
		if (tourXYZ[2] > -14250.0)
		{
			tourXYZ[2] -= 10.0f;
			tourLookXYZ[2] -= 10.0f;
		}
		else
		{
			tourPhase = 3;
		}
		break;
	case 3:
		if (tourCameraAngle > 0)
		{
			tourCameraAngle -= 1.0f;
			tourLookXYZ[0] -= cosd(tourCameraAngle);
			tourLookXYZ[2] += sind(tourCameraAngle);
		}
		else
		{
			tourPhase = 4;
		}
		break;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	if (tour)
	{
		doTourMovement();
		gluLookAt(tourXYZ[0], tourXYZ[1], tourXYZ[2],
			tourLookXYZ[0], tourLookXYZ[1], tourLookXYZ[2],
			0, 1, 0);
	}
	else
	{
		gluLookAt(x, y, z,
			lx, sin(angley* (3.1415926535 / 180)), lz,
			0, cos(angley* (3.1415926535 / 180)), 0);
	}

	lightPosition[0] = currentBodyCenterPositionXZ[curStudent][0];
	lightPosition[1] = 700;
	lightPosition[2] = currentBodyCenterPositionXZ[curStudent][1];
	lightPosition[3] = 1;


	lightDirection[0] = 0;
	lightDirection[1] = -1;
	lightDirection[2] = 0;

	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDirection);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT3, GL_POSITION, tourXYZ);
	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 60.0);
	glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 0.0);
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, tourLookXYZ);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);

	if (switchedOn)
	{
		glEnable(GL_LIGHT3);
	}
	else
	{
		glDisable(GL_LIGHT3);
	}

	float sun[] = { 100, 100, 100 };

	glLightfv(GL_LIGHT2, GL_POSITION, sun);
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, sunlight_diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, sunlight_specular);

	switch (lightMode)
	{
	case LIGHTMODE_AMBIENT:
		//glEnable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		break;
	case LIGHTMODE_SPOTLIGHT:
		//glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
		break;
	case LIGHTMODE_SUN:
		//glEnable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		break;
	case LIGHTMODE_BOTH:
		//glEnable(GL_LIGHT0);
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
		break;
	}
	
	draw_Static_Objects();

	placeStudent();
	placeOthers();

	glutSwapBuffers();
}



void scale(bool increase)
{
	if (increase)
	{
		gscale += 0.25;
	}
	else
	{
		gscale -= 0.25;
	}
	glutPostRedisplay();
}

void rotateLimbs()
{
	if (limbRoatationPositive)
	{
		leftArmRotation += 1;
		rightArmRotation -= 1;
		if (leftArmRotation >= 45.0) 
		{ 
			limbRoatationPositive = false; 
		}
	}
	else
	{
		leftArmRotation -= 1;
		rightArmRotation += 1;
		if (leftArmRotation <= -45.0)
		{
			limbRoatationPositive = true;
		}
	}
}

void move(bool forward)
{
	rotateLimbs();
	if (forward)
	{
		studentBoxes[curStudent].upperLeftX -= sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
		studentBoxes[curStudent].upperLeftY += cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;

		if (!checkForCollisions())
		{

			currentBodyCenterPositionXZ[curStudent][0] -= sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;//X
			currentBodyCenterPositionXZ[curStudent][1] += cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;//Z

			x -= sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;//X
			z += cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;//Z

			lx = currentBodyCenterPositionXZ[curStudent][0];
			lz = currentBodyCenterPositionXZ[curStudent][1];
		}
		else
		{
			studentBoxes[curStudent].upperLeftX += sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
			studentBoxes[curStudent].upperLeftY -= cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;
		}
	}
	else
	{
		
		studentBoxes[curStudent].upperLeftX += sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
		studentBoxes[curStudent].upperLeftY -= cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;

		if (!checkForCollisions())
		{
			currentBodyCenterPositionXZ[curStudent][0] += sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
			currentBodyCenterPositionXZ[curStudent][1] -= cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;

			x += sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
			z -= cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;

			lx = currentBodyCenterPositionXZ[curStudent][0];
			lz = currentBodyCenterPositionXZ[curStudent][1];
		}
		else
		{
			studentBoxes[curStudent].upperLeftX -= sin(-theta[curStudent] * (3.1415926535 / 180)) * 20;
			studentBoxes[curStudent].upperLeftY += cos(-theta[curStudent] * (3.1415926535 / 180)) * 20;
		}
	}
	glutPostRedisplay();
}

void rotate(bool right)
{
	if (!right)
	{
		theta[curStudent] += 2.0;
		if (theta[curStudent] > 360.0)
		{
			theta[curStudent] -= 360.0;
		}
	}
	else
	{
		theta[curStudent] -= 2.0;
		if (theta[curStudent] < -360.0)
		{
			theta[curStudent] += 360.0;
		}
	}
	glutPostRedisplay();
}

void keyboard(int key, int xx, int yy)
{
	float fraction = 0.2f;
	if (!tour)
	{
		switch (key)
		{
		case GLUT_KEY_LEFT:
			angle -= 1.0f;

			x = currentBodyCenterPositionXZ[curStudent][0] + 800 * cos(angle * (3.1415926535 / 180));
			z = currentBodyCenterPositionXZ[curStudent][1] + 800 * sin(angle * (3.1415926535 / 180));
			lx = currentBodyCenterPositionXZ[curStudent][0];
			lz = currentBodyCenterPositionXZ[curStudent][1];
			break;
		case GLUT_KEY_RIGHT:
			angle += 1.0f;

			x = currentBodyCenterPositionXZ[curStudent][0] + 800 * cos(angle * (3.1415926535 / 180));
			z = currentBodyCenterPositionXZ[curStudent][1] + 800 * sin(angle * (3.1415926535 / 180));

			lx = currentBodyCenterPositionXZ[curStudent][0];
			lz = currentBodyCenterPositionXZ[curStudent][1];
			break;
		case GLUT_KEY_UP:
			if (angley > -10)
			{
				angley -= 1.0f;
				y -= 100 * tan(angley * (3.1415926535 / 180));
			}
			break;
		case GLUT_KEY_DOWN:
			if (angley < 70)
			{
				angley += 1.0f;
				y += 100 * tan(angley * (3.1415926535 / 180));
			}
			break;
		}
	}
}

void keys(unsigned char key, int x, int y)
{
	if (key == '=')
	{
		for (int i = 0; i < 3; i++)
		{
			light_ambient[i] += .1;
			light_diffuse[i] += .1;
			light_specular[i] += .1;
			sunlight_diffuse[i] += .1;
			sunlight_specular[i] += .1;
		}
	}
	else if (key == '-')
	{
		for (int i = 0; i < 3; i++)
		{
			light_ambient[i] -= .1;
			light_diffuse[i] -= .1;
			light_specular[i] -= .1;
			sunlight_diffuse[i] -= .1;
			sunlight_specular[i] -= .1;
		}
	}
	else if (key == 'l')
	{
		if (lightMode < 3)
		{
			lightMode++;
		}
		else
		{
			lightMode = 0;
		}
	}
	else if (key == 'x')
	{
		scale(DECREASE);
	}
	else if (key == 'z')
	{
		scale(INCREASE);
	}
	else if (key == 'w')
	{
		move(FORWARD);
	}
	else if (key == 'a')
	{
		rotate(LEFT);
	}
	else if (key == 's')
	{
		move(BACK);
	}
	else if (key == 'd')
	{
		rotate(RIGHT);
	}
	else if (key == 'c')
	{
		curStudent = (curStudent + 1) % 3;
	}
	else if (key == 't')
	{
		if (tour == false)
		{
			tour = true;
		}
		else
		{
			tour = false;
		}
	}
	else if (key == 'f')
	{
		if (!switchedOn)
		{
			switchedOn = true;
		}
		else
			switchedOn = false;
	}
}

void changeSize(int w, int h)
{

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 10000.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

void main(int argc, char **argv)
{
	x = currentBodyCenterPositionXZ[curStudent][0] + 800 * cos(angle * (3.1415926535 / 180));
	z = currentBodyCenterPositionXZ[curStudent][1] + 800 * sin(angle * (3.1415926535 / 180));
	y = 1500;
	lx = currentBodyCenterPositionXZ[curStudent][0];
	lz = currentBodyCenterPositionXZ[curStudent][1];

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800.0, 800.0);
	glutCreateWindow("COMP-5/6400 Assignment 6");

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutKeyboardFunc(keys);
	glutSpecialFunc(keyboard);
	glutReshapeFunc(changeSize);

	myinit();
	
	glutMainLoop();
}
