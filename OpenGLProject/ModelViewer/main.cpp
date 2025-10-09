#include <stdio.h>           
#include <math.h>           
#include <windows.h>         
#include <gl/glut.h>       
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "main.h"
// Global Variables
bool m_fullscreen;				
bool m_culling = false;			// Culling Enabled is Mandatory for this assignment do not change
float m_aspect = 1;

const float m_PI = 3.1415926535897932384626433832795028;
const float m_epsilon = 0.001;


void InitializeWindow(int windowWidth , int windowHeight)
{
	int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
	int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(windowWidth, windowHeight);
	// Center the window
	int centerX = (screenWidth - windowWidth) / 2;
	int centerY = (screenHeight - windowHeight) / 2;
	glutInitWindowPosition(centerX, centerY); // Window Position
}

void InitializeLights(void) {

	glEnable(GL_LIGHT0);	
	glEnable(GL_LIGHTING);							   // Enable Lighting
	GLfloat lightpos[4] = { 0,0,10,0 };
	glLightfv(GL_LIGHT1, GL_POSITION, lightpos);
}


// Our GL Specific Initializations. Returns true On Success, false On Fail.
bool init(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);             // Pixel Storage Mode To Byte Alignment
	glEnable(GL_TEXTURE_2D);                           // Enable Texture Mapping 
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);			   // Gray Background (CHANGED)
    glClearDepth(1.0f);								   // Depth Buffer Setup
    glDepthFunc(GL_LEQUAL);							   // The Type Of Depth Testing To Do
    glEnable(GL_DEPTH_TEST);						   // Enables Depth Testing
    glShadeModel(GL_SMOOTH);						   // Enable Smooth Shading
	InitializeLights();
	glEnable(GL_COLOR_MATERIAL);					   // Enable Material Coloring
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Hint for nice perspective interpolation
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);	// Set the color tracking for both faces for both the ambient and diffuse components
	glEnable(GL_NORMALIZE);
	glFrontFace(GL_CCW);                               //Counter Clock Wise definition of the front and back side of faces
	glCullFace(GL_BACK);                               //Hide the back side
	
	return true;
}

void PositionCamera(){

	glMatrixMode(GL_PROJECTION);     // Select The Projection Matrix

	glLoadIdentity();                // Reset The Projection Matrix
	// Calculate The Aspect Ratio And Set The Clipping Volume
	gluPerspective(45.0f, m_aspect, 0.1, 100.0);

	// Position The Camera to look at the origin 
	gluLookAt(0.0f,0.0f,30.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);

	glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix

}

// Our Reshaping Handler (Required Even In Fullscreen-Only Modes)
void Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	// Calculate The Aspect Ratio And Set The Clipping Volume
	if (h == 0) h = 1;
	m_aspect = (float)w/(float)h;
	PositionCamera();
	glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix
	InitializeLights();
}

// Our Keyboard Handler (Normal Keys)
void KeyboardHandler(unsigned char key, int x, int y)
{
	switch (key) {
		case 'w':
			break;
		case 's':
			break;
		case 'd':
			break;
		case 'a':
			break;
		case 'l':
		{
			break;
		}
		case 'm':
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
void SpecialKeyHandler(int a_keys, int x, int y)
{

	switch (a_keys) {
		case GLUT_KEY_F1:
			m_fullscreen = !m_fullscreen;      
			if (m_fullscreen) 
			{ 
				glutFullScreen();
			} 
			else
			{
				glutReshapeWindow(500, 500);
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

void MouseButtonHandler(int button, int state, int x, int y) {
	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_UP) // state = GLUT_DOWN
		{

		}
		else // state = GLUT_DOWN
		{

		}
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN) // state = GLUT_DOWN
		{
		}
		
	}
}

// Main Function For Bringing It All Together.
int main(int argc, char** argv)
{
	glutInit(&argc, argv);                          
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE); 
	InitializeWindow(1000,1000);
	glutCreateWindow("EPL426"); // Window Title
	if (!init()) {                                   // Our Initialization
		fprintf(stderr,"Initialization failed.");
		return -1;
	}
	glutDisplayFunc(Render);                     // Register The Display Function
	glutReshapeFunc(Reshape);                    // Register The Reshape Handler
	glutKeyboardFunc(KeyboardHandler);                  // Register The Keyboard Handler
	glutSpecialFunc(SpecialKeyHandler);               // Register Special Keys Handler
	glutMouseFunc(MouseButtonHandler);					 // Register Mouse buttons
	glutIdleFunc(NULL);                        	 // We Do Rendering In Idle Time
	glutMainLoop();                              // Go To GLUT Main Loop
	return 0;
}

// Our Rendering Is Done Here
void Render(void)   
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer

	// Do we have culling enabled?
	if (m_culling == true)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glLoadIdentity();


	glLoadIdentity();
    // Swap The Buffers To Make Our Rendering Visible
    glutSwapBuffers();
	glutPostRedisplay(); //animation
}