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

CameraPosition m_camera;

const float m_PI = 3.1415926535897932384626433832795028;
const float m_epsilon = 0.001;

float m_moonAngle = 0.0f; // Angle for moon orbit
float m_ProperllerAngle = 0.0f; // Angle for propeller rotation
float m_DistanceOfPlanets = 20.0f; //Distance for planet orbit

void CreateTheCenterPlanet(void)
{
	// Create the central planet at the origin
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 1.0f); // Blue color for the planet
	glutSolidSphere(5.0, 50, 50); // Radius 2.0, 50 slices and stacks
	glPopMatrix();
}

void CreateTheMoon()
{
	// Create the moon orbiting the planet
	glColor3f(0.5f, 0.5f, 0.5f); // Gray color for the moon
	glutSolidSphere(3.0, 30, 30); // Radius 0.5, 30 slices and stacks
}

void CreatePropeller(float angleDeg, float hubRadius = 0.35f, float bladeSpan = 2.2f, float bladeThickness = 0.3f)
{
	glPushMatrix();

	// Spin around airplane's +X axis
	glRotatef(angleDeg, 1.f, 0.f, 0.f);

	// Blade 1 (move a hair away from hub to avoid z-fighting look)
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.02f);
	glColor3f(0.15f, 0.15f, 0.15f);
	glScalef(bladeThickness, bladeSpan, 0.3f);
	glutSolidCube(1.0);
	glPopMatrix();

	// Blade 2 (90°)
	glPushMatrix();
	glRotatef(90.f, 1.f, 0.f, 0.f);
	glTranslatef(0.0f, 0.0f, 0.02f);
	glScalef(bladeThickness, bladeSpan, 0.3f);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void CreateAirplane(float propAngleDeg)
{

	static const float kBodyRadius = 2.0f;   // base sphere radius
	static const float kBodyScaleX = 3.0f;   // stretch along +X (length)
	static const float kBodyScaleY = 1.0f;   // vertical thickness
	static const float kBodyScaleZ = 1.0f;   // width
	static const float kHalfLength = kBodyRadius * kBodyScaleX;
	static const float kMidX = 0.0f;
	static const float Width = 0.4f; // offset from origin if needed 

	// ===== FUSELAGE: single stretched sphere (ellipsoid) =====
	glPushMatrix();
	glColor3f(0.90f, 0.15f, 0.15f);      // bright red
	glScalef(kBodyScaleX, kBodyScaleY, kBodyScaleZ);
	glutSolidSphere(kBodyRadius, 32, 32);
	glPopMatrix();

	// ===== MAIN WINGS (centered) =====
	glPushMatrix();
	glColor3f(0.10f, 0.25f, 0.90f);      // vivid blue
	glTranslatef(kMidX + 0.6f, 0.0f, 0.0f); // slight forward placement
	glScalef(3.0f, Width, 12.0f);        // chord, thickness, span
	glutSolidCube(1.0);
	glPopMatrix();

	// ===== WING ENGINES + PROPS (both sides) =====
	// Right engine + prop
	glPushMatrix();
	glTranslatef(kMidX + 1.0f, 0.0f, 6.0f);  // out on the wing, slightly forward
	glColor3f(0.85f, 0.85f, 0.88f);
	glutSolidSphere(1.2f, 18, 18);           // nacelle
	glTranslatef(1.2f, 0.0f, 0.0f);          // prop in front
	CreatePropeller(propAngleDeg);
	glPopMatrix();

	// Left engine + prop
	glPushMatrix();
	glTranslatef(kMidX + 1.0f, 0.0f, -6.0f);
	glColor3f(0.85f, 0.85f, 0.88f);
	glutSolidSphere(1.2f, 18, 18);
	glTranslatef(1.2f, 0.0f, 0.0f);
	CreatePropeller(propAngleDeg);
	glPopMatrix();

	// ===== HORIZONTAL STABILIZERS (tail wings) =====
	glPushMatrix();
	glColor3f(0.12f, 0.30f, 0.95f);
	glTranslatef(-kHalfLength - 0.3f, 0.25f, 0.0f); // push clearly behind the fuselage
	glScalef(1.6f, Width, 4.2f);
	glutSolidCube(1.0);
	glPopMatrix();

	// ===== VERTICAL TAIL FIN =====
	glPushMatrix();
	glColor3f(0.10f, 0.85f, 0.20f);
	glTranslatef(-kHalfLength - 0.5f, 1.3f, 0.0f);  // a bit farther back and above
	glScalef(0.28f, 2.0f, 0.6f);                    // tall & thin
	glutSolidCube(1.0);
	glPopMatrix();

	// ===== NOSE PROP (front centerline) =====
	glPushMatrix();
	glTranslatef(+kHalfLength, 0.0f, 0.0f);
	glColor3f(0.85f, 0.85f, 0.88f);
	glutSolidSphere(0.5f, 18, 18);
	glTranslatef(0.6f, 0.0f, 0.0f);  // in front of nose so it’s clearly visible
	CreatePropeller(propAngleDeg);
	glPopMatrix();

}

void CreateSun()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float baseRadius = 10.0f;
	float baseAlpha = 1.0f;

		for(int i=0; i<4; i++)
		{
			float currentRadius = baseRadius + i * 2.0f; // Increase radius for each layer
			float currentAlpha = baseAlpha - i * 0.2f;   // Decrease alpha for each layer
			glColor4f(1.0f, 1.0f, 0.0f, currentAlpha); // Yellow color with varying alpha
			glutSolidSphere(currentRadius, 50, 50); // Draw sphere with current radius
		}
}


void PositionSun(float angle)
{
	// Create the moon orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(-m_DistanceOfPlanets *1.5, 0.0f, 0.0f); // Position the moon 5 units away from the planet
	CreateSun();
	glPopMatrix();
}



void PositionMoon(float angle)
{
	// Create the moon orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(m_DistanceOfPlanets, 0.0f, 0.0f); // Position the moon 5 units away from the planet
	CreateTheMoon();
	glPopMatrix();

}

void PositionAirplane(float angle)
{
	// Create the moon orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(10.0f, 0.0f, 0.0f); // Position the airplane 5 units away from the planet
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f); // Rotate to face forward along +X axis
	glScalef(0.2f, 0.2f, 0.2f); // Scale down the airplane
	CreateAirplane(m_ProperllerAngle);
	glPopMatrix();
}

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

void PositionCamera()
{

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, m_aspect, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);

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
			m_camera.m_pos = vector3d(-20.0f, 0.0f, 0.0f);
			m_camera.m_view = vector3d(20.0f, 0.0f, 0.0f); // look at origin
			m_camera.m_up = vector3d(0.0f, 1.0f, 0.0f);   // Y is up
			break;
		case 's':
			m_camera.m_pos = vector3d(0.0f, 0.0f, 150.0f);
			m_camera.m_view = vector3d(0.0f, 0.0f, 0.0f);
			m_camera.m_up = vector3d(0.0f, 1.0f, 0.0f);   // Y is up
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

void RenderCameraView()
{
	// Position The Camera to look at the origin 
	gluLookAt(m_camera.m_pos.x(), m_camera.m_pos.y(), m_camera.m_pos.z(),
			  m_camera.m_view.x(), m_camera.m_view.y(), m_camera.m_view.z(),
			  m_camera.m_up.x(), m_camera.m_up.y(), m_camera.m_up.z());
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

void RenderScene()
{
	m_moonAngle = m_moonAngle + 0.1;
	m_ProperllerAngle = m_ProperllerAngle + 0.5;
	// Draw the scene
	CreateTheCenterPlanet();
	PositionMoon(m_moonAngle);
	PositionAirplane(m_moonAngle);
	PositionSun(m_moonAngle);
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
	RenderCameraView();
	RenderScene();

    // Swap The Buffers To Make Our Rendering Visible
    glutSwapBuffers();
	glutPostRedisplay(); //animation
}