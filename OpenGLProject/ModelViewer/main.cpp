#include <stdio.h>           // Standard C/C++ Input-Output
#include <math.h>            // Math Functions
#include <windows.h>         // Standard Header For MSWindows Applications
#include <gl/glut.h>            // The GL Utility Toolkit (GLUT) Header
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct vector3d
{
	float X, Y, Z;

	inline vector3d(void) {}
	inline vector3d(const float x, const float y, const float z)
	{
		X = x; Y = y; Z = z;
	}

	inline vector3d operator + (const vector3d& A) const
	{
		return vector3d(X + A.X, Y + A.Y, Z + A.Z);
	}

	inline vector3d operator + (const float A) const
	{
		return vector3d(X + A, Y + A, Z + A);
	}

	inline float Dot(const vector3d& A) const
	{
		return A.X * X + A.Y * Y + A.Z * Z;
	}
};

// The Following Directive Fixes The Problem With Extra Console Window
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

// Uncomment the following if you want to use the glut library from the current directory
//#pragma comment(lib, "lib/glut32.lib")

// Global Variables
bool g_gamemode;				// GLUT GameMode ON/OFF
bool g_fullscreen;				// Fullscreen Mode ON/OFF (When g_gamemode Is OFF)
bool b_culling = false;			// Culling Enabled is Mandatory for this assignment do not change
float aspect = 1;

const float PI = 3.1415926535897932384626433832795028;
const float epsilon = 0.001;

//Function Prototypes
void render(void);
void initLights(void);
bool init(void);
void reshape(int w,int h);
void keyboard(unsigned char key, int x, int y);
void special_keys(int a_keys, int x, int y);
void drawAxes();
void movingLight();

float angle = 0;
float temp = 0;
float temp2 = 0;
int temp3 = 0;
bool movingLightFlag = false;

void initLights(void) {

	glEnable(GL_LIGHT0);	
	//glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);							   // Enable Lighting
	//glDisable(GL_LIGHTING);

	GLfloat lightpos[4] = { 0,0,10,0 };
	glLightfv(GL_LIGHT1, GL_POSITION, lightpos);
}


// Our GL Specific Initializations. Returns true On Success, false On Fail.
bool init(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);             // Pixel Storage Mode To Byte Alignment
	glEnable(GL_TEXTURE_2D);                           // Enable Texture Mapping 
    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);			   // Gray Background (CHANGED)
    glClearDepth(1.0f);								   // Depth Buffer Setup
    glDepthFunc(GL_LEQUAL);							   // The Type Of Depth Testing To Do
    glEnable(GL_DEPTH_TEST);						   // Enables Depth Testing
    glShadeModel(GL_SMOOTH);						   // Enable Smooth Shading
	initLights();
	glEnable(GL_COLOR_MATERIAL);					   // Enable Material Coloring
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Hint for nice perspective interpolation
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);	// Set the color tracking for both faces for both the ambient and diffuse components
	
	//// Set Specular
	//GLfloat matSpec[] = { 0.1, 0.1,0.1,1 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
	//GLfloat shininess[] = { 64 };
	//glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
	glEnable(GL_NORMALIZE);
	glFrontFace(GL_CCW);                               //Counter Clock Wise definition of the front and back side of faces
	glCullFace(GL_BACK);                               //Hide the back side
	
	return true;
}

void positionCamera(){

	glMatrixMode(GL_PROJECTION);     // Select The Projection Matrix

	glLoadIdentity();                // Reset The Projection Matrix
	gluPerspective(45.0f, aspect, 0.1, 100.0);
	gluLookAt(0.0f,0.0f,30.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f);
	//gluLookAt(10.0f,- 10.0f, 60.0f, 10.0f, -10.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	
	//camera transformations go here
	glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix

}

// Our Reshaping Handler (Required Even In Fullscreen-Only Modes)
void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	// Calculate The Aspect Ratio And Set The Clipping Volume
	if (h == 0) h = 1;
	aspect = (float)w/(float)h;
	positionCamera();
	glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix
	initLights();
}

// Our Keyboard Handler (Normal Keys)
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 'w':
			break;
		case 's':
			break;
		case 'd':
			temp += 1;
			break;
		case 'a':
			temp -= 1;
			break;
		case 'l':
		{
			temp3++;
			if (temp3 > 7) temp3 = 1;
			GLfloat lightOn[4] = { 1,1,1,1 };
			GLfloat lightAmbientOn[4] = { 0.1,0.1,0.1,1 };
			GLfloat lightOff[4] = { 0,0,0,0 };
			switch (temp3)
			{
				case 1:
				{
					printf("AMBIENT only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOn);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOff);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOff);
					break;
				}
				case 2:
				{
					printf("DIFFUSE only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightOff);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOn);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOff);
					break;
				}
				case 3:
				{
					printf("SPECULAR only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightOff);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOff);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOn);
					break;
				}
				case 4:
				{
					printf("AMBIENT & DIFFUSE only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOn);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOn);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOff);
					break;
				}
				case 5:
				{
					printf("AMBIENT & SPECULAR only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOn);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOff);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOn);
					break;
				}
				case 6:
				{
					printf("DIFFUSE & SPECULAR only\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightOff);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOn);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOn);
					break;
				}
				case 7:
				{
					printf("AMBIENT & DIFFUSE & SPECULAR\n");
					glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOn);
					glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOn);
					glLightfv(GL_LIGHT1, GL_SPECULAR, lightOn);
					break;
				}
			}
			break;
		}
		case 'm':
			movingLight();
			break;
		case 27:        // When Escape Is Pressed...
			exit(0);    // Exit The Program
			break;      // Ready For Next Case
		default:
		break;
	}
	glutPostRedisplay();
}

// Our Keyboard Handler For Special Keys (Like Arrow Keys And Function Keys)
void special_keys(int a_keys, int x, int y)
{

	switch (a_keys) {
		case GLUT_KEY_F1:
			// We Can Switch Between Windowed Mode And Fullscreen Mode Only
			if (!g_gamemode) {
				g_fullscreen = !g_fullscreen;       // Toggle g_fullscreen Flag
				if (g_fullscreen) glutFullScreen(); // We Went In Fullscreen Mode
				else glutReshapeWindow(500, 500);   // We Went In Windowed Mode
			}
		break;
		case GLUT_KEY_UP:

			break;
		case GLUT_KEY_DOWN:

			break;
		case GLUT_KEY_LEFT:
			break;
		case GLUT_KEY_RIGHT:
			break;
		default:
			;
	}

	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) // state = GLUT_DOWN
		{
			temp2 = 0;
			//Do something
		}
		else // state = GLUT_DOWN
		{
			temp2 = 45;
			// Do something Else
		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN) // state = GLUT_DOWN
		{
			temp2 -= 1;
			//Do something
		}
		
	}
}

void mouseMove(int x, int y) {
	// Do what ever you want here, x & y is the position of the mouse
	printf("Mouse position is: %i in X axis and %i in Y axis\n", x, y);
}

void movingLight() {
	movingLightFlag = !movingLightFlag;

	if (movingLightFlag)
	{
		printf("Moving light enable\n");
		glEnable(GL_LIGHT1);
		// Set Specular
		GLfloat matSpec[] = { 0.1, 0.1,0.1,1 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
		GLfloat shininess[] = { 64 };
		glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
		GLfloat lightOn[4] = { 1,1,1,1 };
		GLfloat lightAmbientOn[4] = { 0.1,0.1,0.1,1 };
		glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOn);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOn);
		glLightfv(GL_LIGHT1, GL_SPECULAR, lightOn);
	}
	else
	{
		printf("Moving light disable\n");
		glDisable(GL_LIGHT1);
		// Set Specular
		GLfloat matSpec[] = { 0, 0,0,1 };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
	}
	
}

// Main Function For Bringing It All Together.
int main(int argc, char** argv)
{
	glutInit(&argc, argv);                           // GLUT Initializtion
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE); // (CHANGED)|
	if (g_gamemode) {
		glutGameModeString("1024x768:32");            // Select 1024x768 In 32bpp Mode
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
			glutEnterGameMode();                     // Enter Full Screen
		else
			g_gamemode = false;                     // Cannot Enter Game Mode, Switch To Windowed
	}
	if (!g_gamemode) {
		glutInitWindowPosition(100, 100); // Window Position
		glutInitWindowSize(500, 500); // Window Size If We Start In Windowed Mode
		glutCreateWindow("EPL426"); // Window Title
		//int screenSize[] = { glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT) };
		//int windowsSize[] = { 1000,1000 };
		//glutInitWindowPosition((screenSize[0]/2) - (windowsSize[0]/2), (screenSize[1] / 2) - (windowsSize[1] / 2));
		//glutInitWindowSize(windowsSize[0], windowsSize[1]);
		//glutCreateWindow("EPL426 - 1st Assignment");
	}
	if (!init()) {                                   // Our Initialization
		fprintf(stderr,"Initialization failed.");
		return -1;
	}
	glutDisplayFunc(render);                     // Register The Display Function
	glutReshapeFunc(reshape);                    // Register The Reshape Handler
	glutKeyboardFunc(keyboard);                  // Register The Keyboard Handler
	glutSpecialFunc(special_keys);               // Register Special Keys Handler
	glutMouseFunc(mouseButton);					 // Register Mouse buttons
	glutMotionFunc(mouseMove);					 // Register Mouse motion
	glutIdleFunc(NULL);                        	 // We Do Rendering In Idle Time
	glutMainLoop();                              // Go To GLUT Main Loop
	return 0;
}

void RenderSquare(vector3d center, float size)
{
	float halfSize = size/2;

	glBegin(GL_TRIANGLES);
		//triangle 1
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(center.X + halfSize, center.Y + halfSize, center.Z);
		glVertex3f(center.X + halfSize, center.Y - halfSize, center.Z);
		glVertex3f(center.X - halfSize, center.Y + halfSize, center.Z);

		//triangle 2
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f(center.X - halfSize, center.Y + halfSize, center.Z);
		glVertex3f(center.X - halfSize, center.Y - halfSize, center.Z);
		glVertex3f(center.X + halfSize, center.Y - halfSize, center.Z);
	glEnd();
}

void RenderSquare2(vector3d point1, vector3d point2, vector3d point3, vector3d point4, vector3d color)
{
	glColor3f(color.X, color.Y, color.Z);
	glBegin(GL_TRIANGLES);
		//triangle 1
		glVertex3f(point1.X, point1.Y, point1.Z);
		glVertex3f(point2.X, point2.Y, point2.Z);
		glVertex3f(point3.X, point3.Y, point3.Z);

		//triangle 2
		glVertex3f(point2.X, point2.Y, point2.Z);
		glVertex3f(point3.X, point3.Y, point3.Z);
		glVertex3f(point4.X, point4.Y, point4.Z);
	glEnd();
}

void RenderCube(vector3d center, float size)
{
	float halfSize = size / 2;

	// Front
	vector3d frontLeftUp = center + vector3d(-halfSize, halfSize, halfSize);
	vector3d frontRightUp = center + vector3d(halfSize, halfSize, halfSize);
	vector3d frontLeftDown = center + vector3d(-halfSize, -halfSize, halfSize);
	vector3d frontRightDown = center + vector3d(halfSize, -halfSize, halfSize);

	// Back
	vector3d backLeftUp = center + vector3d(-halfSize, halfSize, -halfSize);
	vector3d backRightUp = center + vector3d(halfSize, halfSize, -halfSize);
	vector3d backLeftDown = center + vector3d(-halfSize, -halfSize, -halfSize);
	vector3d backRightDown = center + vector3d(halfSize, -halfSize, -halfSize);

	// Front
	RenderSquare2(frontLeftUp, frontRightUp, frontLeftDown, frontRightDown, vector3d(1,0,0));
	// Right
	RenderSquare2(frontRightUp, frontRightDown, backRightUp, backRightDown, vector3d(1, 1, 0));
	// Back
	RenderSquare2(backLeftUp, backRightUp, backLeftDown, backRightDown, vector3d(0, 1, 0));
	// Left
	RenderSquare2(frontLeftUp, frontLeftDown, backLeftUp, backLeftDown, vector3d(0, 1, 1));
	// Up
	RenderSquare2(frontLeftUp, frontRightUp, backLeftUp, backRightUp, vector3d(0, 0, 1));
	// Down 
	RenderSquare2(frontLeftDown, frontRightDown, backLeftDown, backRightDown, vector3d(1, 1, 1));
}

void  StickMan() {
	// Body
	glPushMatrix();
	glTranslatef(0, 2, 0);
	glutSolidCube(4);

	// Hand L
	glPushMatrix();
	glTranslatef(-(2.5f + 2), 2, 0);
	glScaled(5, 1, 1);
	glutSolidCube(1);
	glPopMatrix();

	// Hand R
	glPushMatrix();
	glTranslatef((2.5f + 2), 2, 0);
	glScaled(5, 1, 1);
	glutSolidCube(1);
	glPopMatrix();

	// Head
	glPushMatrix();
	glTranslatef(0, 4, 0);
	glutSolidSphere(1, 20, 20);
	glPopMatrix();
	glPopMatrix();

	// Leg L
	glPushMatrix();
	glTranslatef(-2, -(3), 0);
	glScaled(1, 6, 1);
	glutSolidCube(1);
	glPopMatrix();

	// Leg R
	glPushMatrix();
	glTranslatef(2, -(3), 0);
	glScaled(1, 6, 1);
	glutSolidCube(1);
	glPopMatrix();
}

// Our Rendering Is Done Here
void render(void)   
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

	// Do we have culling enabled?
	if (b_culling == true)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	glLoadIdentity();

	// Make and rotating light
	if (movingLightFlag) 
	{
		angle = angle + 0.1;
		glm::mat4 myMatrix = 
		{ 
			cos(angle),0,sin(angle),0,
			0,1,0,0,
			-sin(angle),0,cos(angle),0,
			0,0,0,1 
		};

		glm::vec4 myVector = { 0.0f, 0.0f, -4.0f, 1 };
		glm::vec4 transformedVector = myMatrix * myVector; 
		GLfloat lightpos[4] = { transformedVector[0] ,transformedVector[1] ,transformedVector[2] ,transformedVector[3] };
		glLightfv(GL_LIGHT1, GL_POSITION, lightpos);
		//printf("%f , %f, %f, %f\n", myMatrix[0][0], myMatrix[0][1], myMatrix[1][0], myMatrix[1][1]);
		//printf("%f , %f, %f, %f\n", transformedVector[0], transformedVector[1], transformedVector[2], transformedVector[3]);
		glColor3f(1.0f, 0.0f, 0.0f); // red
		glTranslatef(transformedVector[0], transformedVector[1], transformedVector[2]);
		glutSolidCube(1);
	}

	glLoadIdentity();

#pragma region Translation

	// *** Move all ***
	// Translate 5 on the right
	//glTranslatef(5, 0, 0);
	//glBegin(GL_TRIANGLES);
	//	//triangle 1
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, 1, 0);
	//	glVertex3f(-1, 0, 0);
	//	//triangle 2
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, -1, 0);
	//	glVertex3f(-1, 0, 0);
	//glEnd();

	// *** Move seperate ***

	//glColor3f(0.0f, 1.0f, 0.0f); // green
	//glTranslatef(5, 0, 0);
	//glBegin(GL_TRIANGLES);
	//	//triangle 1
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, 1, 0);
	//	glVertex3f(-1, 0, 0);
	//glEnd();
	//glColor3f(1.0f, 0.0f, 0.0f); // red
	//glTranslatef(0, -2, 0);
	//glBegin(GL_TRIANGLES);
	//	//triangle 2
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, -1, 0);
	//	glVertex3f(-1, 0, 0);
	//glEnd();

#pragma endregion

#pragma region Rotetion
	//// *** Rotate on zero around of z axis ***
	//glRotatef(1, 0, 0, 1);
	//RenderSquare(vector3d(0, 0, 0), 2);

	//// *** Rotate after translation around of z axis ***
	//glRotatef(45, 0, 0, 1);
	//glTranslatef(0, 5, 0);
	//RenderSquare(vector3d(0, 0, 0), 2);
#pragma endregion

#pragma region Scale
	//// *** Scale on zero on x axis ***
	//glScalef( 2, 1, 1);
	//RenderSquare(vector3d(0, 0, 0), 2);

	//// *** Scale after translation around on x axis ***
	//glScalef(2, 1, 1);
	//glTranslatef(2, 0, 0);
	////glScalef(2, 1, 1);
	//RenderSquare(vector3d(0, 0, 0), 2);
#pragma endregion

#pragma region Matrix
	//// *** Move with translate ***
	//glTranslatef(2, 0, 0);
	//RenderSquare(vector3d(0, 0, 0), 2);

	//// *** Move with matrix ***
	//float f[16] =
	//{
	//	1,0,0,0,
	//	0,1,0,0,
	//	0,0,1,0,
	//	2,0,0,1
	//};
	//glLoadMatrixf(f);
	//RenderSquare(vector3d(0, 0, 0), 2);

#pragma endregion

#pragma region Push and Pop

	// *** 1 ***
	//glPushMatrix(); 
	//glColor3f(0.0f, 1.0f, 0.0f); // green
	//glutSolidCube(1); 
	//glTranslatef(2, 0, 0); 
	//glColor3f(1.0f, 0.0f, 0.0f); // red
	//glutSolidCube(1); 
	//glTranslatef(2, 0, 0); 
	//glColor3f(0.0f, 0.0f, 1.0f); // blue
	//glutSolidCube(1); 
	//glTranslatef(0, 2, 0); 
	//glColor3f(1.0f, 1.0f, 0.0f); // yellow
	//glutSolidCube(1);

	//// *** 2 ***
	//glPushMatrix(); 
	//	glColor3f(0.0f, 1.0f, 0.0f); // green
	//	glutSolidCube(1); 
	//	glTranslatef(2, 0, 0); 
	//	glColor3f(1.0f, 0.0f, 0.0f); // red
	//	glutSolidCube(1); 
	//	glTranslatef(2, 0, 0); 
	//	glColor3f(0.0f, 0.0f, 1.0f); // blue
	//	glutSolidCube(1); 
	//glPopMatrix(); 
	//glTranslatef(0, 2, 0); 
	//glColor3f(1.0f, 1.0f, 0.0f); // yellow
	//glutSolidCube(1);

	//// *** 3 ***
	//glColor3f(0.0f, 1.0f, 0.0f); // green
	//glutSolidCube(1); 
	//glTranslatef(2, 0, 0); 
	//glColor3f(1.0f, 0.0f, 0.0f); // red
	//glutSolidCube(1); 
	//glPushMatrix(); 
	//	glTranslatef(2, 0, 0); 
	//	glColor3f(0.0f, 0.0f, 1.0f); // blue
	//	glutSolidCube(1); 
	//glPopMatrix(); 
	//glLoadIdentity();
	//glTranslatef(0, 2, 0); 
	//glColor3f(1.0f, 1.0f, 0.0f); // yellow
	//glutSolidCube(1);

#pragma endregion


#pragma region StickMan

	glColor3f(0.0f, 1.0f, 0.0f);
	glTranslatef(temp, 0, 0);
	glRotatef(temp2, 0, 1, 0);
	StickMan();

#pragma endregion

#pragma region Other
	//glColor3f(0.0f, 1.0f, 0.0f);

	//// *** Original gode ***
	//glBegin(GL_TRIANGLES);
	//	//triangle 1
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, 1, 0);
	//	glVertex3f(-1, 0, 0);
	//	//triangle 2
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(0, -1, 0);
	//	glVertex3f(-1, 0, 0);
	//glEnd();

	// *** Extended original code ***

	//float sizeX = 2.0f;
	//float sizeY = 3.0f;

	//glBegin(GL_TRIANGLES);
	//	//triangle 1
	//	glVertex3f(sizeX, sizeY,0);
	//	glVertex3f(0, sizeY,0);
	//	glVertex3f(sizeX,0,0);

	//	//glColor3f(1.0f, 1.0f, 0.0f);
	//	//triangle 2
	//	glVertex3f(0, sizeY,0);
	//	glVertex3f(0,0,0);
	//	glVertex3f(sizeX,0,0);
	//glEnd();
	
	// *** Other functions ***

	//RenderSquare(vector3d(2, 0, -10),10);

	//RenderCube(vector3d(0, 0, 0), 2);
#pragma endregion

    // Swap The Buffers To Make Our Rendering Visible
    glutSwapBuffers();
	glutPostRedisplay(); //animation
}