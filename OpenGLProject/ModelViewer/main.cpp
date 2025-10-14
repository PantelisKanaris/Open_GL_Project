#include <stdio.h>           
#include <math.h>           
#include <windows.h>         
#include <gl/glut.h>       
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "main.h"
#include "stb_image.h"
#include "MyMath.h"

#define STB_IMAGE_IMPLEMENTATION
// Global Variables
bool m_fullscreen;				
bool m_culling = false;			// Culling Enabled is Mandatory for this assignment do not change
float m_aspect = 1;

CameraPosition m_camera;

const float m_PI = 3.1415926535897932384626433832795028;
const float m_epsilon = 0.001;

float m_PlanetAngle = 0.0f; // Angle for the planets orbit
float m_AirplaneAngle = 0.0f; // Angle for airplane movement
float m_ProperllerAngle = 0.0f; // Angle for propeller rotation
float m_DistanceOfPlanets = 20.0f; //Distance for planet orbit
float m_PlanetSpeed = 1.0f; // Speed of moon orbit
float m_airplaneSpeed = 0.2f; // Speed of airplane movement
float m_propellerSpeed = 5.0f; // Speed of propeller rotation
bool m_downKeyState = false; // Control key state
int m_sunPositionMultyplier = 2; // Multiplier for sun position
GLuint m_gEarthDayTex = 0;
GLuint m_gEarthNightTex = 0;
GLuint m_gMoonTex = 0;
GLuint m_gSunTex = 0;
GLUquadric* m_gEarthQuad = nullptr;
GLUquadric* m_gMoonQuad = nullptr;
GLUquadric* m_gSunQuad = nullptr;


GLuint LoadTexture(const char* filename)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
	if (!data) {
		printf("Failed to load image: %s\n", filename);
		return 0;
	}

	GLenum format = GL_RGB;
	if (channels == 1)      format = GL_RED;
	else if (channels == 3) format = GL_RGB;
	else if (channels == 4) format = GL_RGBA;

	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	return texID;
}

void InitializeEarthTexture()
{
	stbi_set_flip_vertically_on_load(1);
	m_gEarthDayTex = LoadTexture("Textures/earth_2k.jpg");
	m_gEarthNightTex = LoadTexture("Textures/earth_2k_night.jpg");
	if (!m_gEarthDayTex || !m_gEarthNightTex) 
	{
		fprintf(stderr, "Earth textures missing\n");
	}
	m_gEarthQuad = gluNewQuadric();
	gluQuadricTexture(m_gEarthQuad, GL_TRUE);   // enable texture coords
	gluQuadricNormals(m_gEarthQuad, GLU_SMOOTH);

}

void InitializeMoonTexture()
{
	stbi_set_flip_vertically_on_load(1);
	m_gMoonTex = LoadTexture("Textures/2k_moon.jpg");
	if (!m_gMoonTex)
	{
		fprintf(stderr, "Moon texture missing\n");
	}
	m_gMoonQuad = gluNewQuadric();
	gluQuadricTexture(m_gMoonQuad, GL_TRUE);  
	gluQuadricNormals(m_gMoonQuad, GLU_SMOOTH);
	glBindTexture(GL_TEXTURE_2D, m_gMoonTex);
}

void InitializeSunTexture()
{
	stbi_set_flip_vertically_on_load(1);  
	m_gSunTex = LoadTexture("Textures/2k_sun.jpg"); 
	if (!m_gSunTex) {
		fprintf(stderr, "Sun texture missing\n");
	}
	m_gSunQuad = gluNewQuadric();
	gluQuadricTexture(m_gSunQuad, GL_TRUE);
	gluQuadricNormals(m_gSunQuad, GLU_SMOOTH);
}

// Returns day factor in [0,1] based on sun Y 
float GetDayFactor()
{
	float radians = m_PlanetAngle * (m_PI / 180.0f);
	float sunY = -sinf(radians) * m_DistanceOfPlanets * m_sunPositionMultyplier;
	float maxD = m_DistanceOfPlanets * m_sunPositionMultyplier;
	float t = (sunY + maxD) / (2.0f * maxD); // 0..1
	// smooth a bit
	t = t * t * (3.f - 2.f * t);
	return t;
}

void CreatePlanetUsingTexture()
{
	// 0..1 where 1 = day, 0 = night, already smoothed in GetDayFactor()
	float day = GetDayFactor();

	// Optional extra smoothing (feather the switch region a bit more)
	auto smooth = [](float x, float a, float b) {
		float t = (x - a) / (b - a);
		if (t < 0.f) t = 0.f; else if (t > 1.f) t = 1.f;
		return t * t * (3.f - 2.f * t);
		};
	float dayW = smooth(day, 0.35f, 0.65f);  // tweak a/b to taste
	float nightW = 1.0f - dayW;
	nightW *= 0.9f; // usually looks better a bit dimmer

	glPushMatrix();

	// ===== PASS 1: Day texture (lit), writes depth =====
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gEarthDayTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glColor3f(dayW, dayW, dayW);               // scale contribution
	if (m_gEarthQuad) gluSphere(m_gEarthQuad, 5.0, 96, 96);

	// ===== PASS 2: Night texture (unlit), additive over the first pass =====
	glBindTexture(GL_TEXTURE_2D, m_gEarthNightTex);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);               // additive
	glDepthMask(GL_FALSE);                     // don’t modify depth
	glDepthFunc(GL_EQUAL);                     // draw exactly where pass 1 drew

	glColor3f(nightW, nightW, nightW);         // scale night contribution
	if (m_gEarthQuad) gluSphere(m_gEarthQuad, 5.0, 96, 96);

	// ---- restore render state ----
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_LIGHTING);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void CreateTheMoonWithTexture()
{
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gMoonTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Use white so the texture’s real colors show
	glColor3f(1.f, 1.f, 1.f);

	if (m_gMoonQuad) 
	{
		gluSphere(m_gMoonQuad, 3.0, 48, 48);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void CreateLightOfSun()
{
	// Calculate the global position of the sun 
	float radians = m_PlanetAngle * (m_PI / 180.0f);
	float sunX = -cos(radians) * m_DistanceOfPlanets * m_sunPositionMultyplier;
	float sunY = -sin(radians) * m_DistanceOfPlanets * m_sunPositionMultyplier;
	float sunZ = 0.0f;

	glEnable(GL_LIGHT1);

	// Set light position
	GLfloat lightPosition[4] = { sunX, sunY, sunZ, 1.0f };
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);

	// Calculate brightness based on sun's vertical position (Y coordinate)
	// When sun is at top (positive Y): bright
	// When sun is at bottom (negative Y): dim
	float maxDistance = m_DistanceOfPlanets * m_sunPositionMultyplier;
	float brightnessMultiplier = (sunY + maxDistance) / (2.0f * maxDistance); // Normalize to [0, 1]
	
	
	brightnessMultiplier = 0.05f + (brightnessMultiplier * 4.95f);
	
	// Base light intensities
	float baseAmbient = 0.1f;
	float baseDiffuseR = 0.8f;
	float baseDiffuseG = 0.7f; 
	float baseDiffuseB = 0.5f;
	float baseSpecular = 0.6f;
	
	// Apply brightness multiplier to light properties
	GLfloat lightAmbient[4] = { 
		baseAmbient * brightnessMultiplier, 
		baseAmbient * brightnessMultiplier, 
		baseAmbient * brightnessMultiplier, 
		1.0f 
	};
	GLfloat lightDiffuse[4] = { 
		baseDiffuseR * brightnessMultiplier, 
		baseDiffuseG * brightnessMultiplier, 
		baseDiffuseB * brightnessMultiplier, 
		1.0f 
	};
	GLfloat lightSpecular[4] = { 
		baseSpecular * brightnessMultiplier, 
		baseSpecular * brightnessMultiplier, 
		baseSpecular * brightnessMultiplier, 
		1.0f 
	};

	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);

	// Minimal attenuation for maximum visibility
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0f);
	
	//glPushMatrix();
	//glTranslatef(sunX, sunY, sunZ);
	//glColor3f(1.0f, 1.0f, 1.0f); // Bright white marker
	//glutSolidSphere(1.0f, 10, 10); // Small marker sphere
	//glPopMatrix();
}

void CreateLightOfAirplane()
{
	// Calculate the global position of the airplane (same logic as PositionAirplane)
	float radians = m_AirplaneAngle * (m_PI / 180.0f);
	float airplaneX = cos(radians) * 10.0f; // 10.0f is the distance from center
	float airplaneY = sin(radians) * 10.0f;
	float airplaneZ = 0.0f;

	// Enable the airplane light
	glEnable(GL_LIGHT2);

	// Set light position at the center of the airplane
	GLfloat lightPosition[4] = { airplaneX, airplaneY, airplaneZ, 1.0f };
	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition);

	// Configure airplane light properties (white/blue-ish for navigation lights)
	GLfloat lightAmbient[4] = { 0.05f, 0.05f, 0.1f, 1.0f };   // Slight blue ambient
	GLfloat lightDiffuse[4] = { 0.8f, 0.9f, 1.0f, 1.0f };     // Cool white light
	GLfloat lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };    // Bright specular

	glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular);

	// Set moderate attenuation for airplane light
	glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.00f);
	glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.00f);

	// Optional: Debug marker to see where the light is positioned
	//glPushMatrix();
	//glTranslatef(airplaneX, airplaneY, airplaneZ);
	//glColor3f(0.0f, 1.0f, 1.0f); // Cyan marker
	//glutSolidSphere(0.5f, 8, 8); // Small marker sphere
	//glPopMatrix();
}

void ChangeColourOfBackground()
{
	const vector3d night = { 0.0f, 0.0f, 0.0f };

	// Much darker day color
	const vector3d day = { 0.25f, 0.45f, 0.7f };

	float normalizedAngle = fmod(m_PlanetAngle, 360.0f) / 360.0f; // Normalize angle to [0, 1]

	// Created a sine wave pattern so it goes: night → day → night
	float radians = normalizedAngle * 2.0f * m_PI; // Convert to radians
	float t = (-sin(radians) + 1.0f) / 2.0f; 

	// Apply smoothstep for smoother transitions
	t = t * t * (3.0f - 2.0f * t);

	// Interpolate between night and day colors for each RGB component
	float r = LinearInterpolation(night.x(), day.x(), t);
	float g = LinearInterpolation(night.y(), day.y(), t);
	float b = LinearInterpolation(night.z(), day.z(), t);

	// Set the background color
	glClearColor(r, g, b, 1.0f);
}

void CreateTheCenterPlanet(void)
{
	// Create the central planet at the origin - more realistic earth-like blue
	glPushMatrix();
	glColor3f(0.2f, 0.4f, 0.7f); // Desaturated blue (was 0.0, 0.0, 1.0)
	glutSolidSphere(5.0, 50, 50);
	glPopMatrix();
}

void CreateTheMoon()
{
	// Create the moon orbiting the planet - darker, more realistic gray
	glColor3f(0.25f, 0.25f, 0.25f); // Much darker gray (was 0.5, 0.5, 0.5)
	glutSolidSphere(3.0, 30, 30);
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

		glDisable(GL_BLEND); 
}

void CreateSunWithTexture()
{
	const float coreR = 10.0f;  // your previous baseRadius

	glPushMatrix();

	// ===== CORE: textured, emissive (unlit) =====
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_gSunTex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_LIGHTING);     // self-lit look
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glColor4f(1.0f, 0.8f, 0.3f, 1.0f);
	if (m_gSunQuad) 
	{ 
		gluSphere(m_gSunQuad, coreR, 64, 64); 
	}

	// ===== OUTER GLOW: gentle translucent shells =====
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // soft fade instead of additive
	glDepthMask(GL_FALSE);

	glBindTexture(GL_TEXTURE_2D, 0); // color-only glow

	for (int i = 1; i <= 3; ++i) 
	{
		float r = coreR + i * 2.0f;               // larger radius
		float a = 0.35f - i * 0.1f;               // fade alpha
		if (a < 0.f) a = 0.f;
		glColor4f(1.0f, 0.9f, 0.2f, a);           // warm yellowish glow
		glutSolidSphere(r, 40, 40);
	}

	// ===== restore state =====
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glEnable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void PositionSun(float angle)
{
	// Create the sun orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(-m_DistanceOfPlanets * m_sunPositionMultyplier, 0.0f, 0.0f); // Position the sun double the distance away from the planet form the moon.
	CreateSun();
	glPopMatrix();
}

void PositionSunWithTexture(float angle)
{
	// Create the sun orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(-m_DistanceOfPlanets * m_sunPositionMultyplier, 0.0f, 0.0f); // Position the sun double the distance away from the planet form the moon.
	CreateSunWithTexture();
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

void PositionMoonWithTexture(float angle)
{
	// Create the moon orbiting the planet
	glPushMatrix();
	glRotatef(angle, 0.0f, 0.0f, 1.0f); // Rotate around the planet
	glTranslatef(m_DistanceOfPlanets, 0.0f, 0.0f); // Position the moon 5 units away from the planet
	CreateTheMoonWithTexture();
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

	/*glEnable(GL_LIGHT0);*/
	glEnable(GL_LIGHTING);
	
	//// Configure GL_LIGHT0 with moderate default lighting
	//GLfloat light0Position[4] = { 0.0f, 0.0f, 1.0f, 0.0f }; // Directional light from front
	//GLfloat light0Ambient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };   // Low ambient
	//GLfloat light0Diffuse[4] = { 0.6f, 0.6f, 0.6f, 1.0f };   // Moderate diffuse
	//GLfloat light0Specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };  // Moderate specular

	//glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
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
	InitializeEarthTexture();
	InitializeMoonTexture();
	InitializeSunTexture();
	return true;
}

void PositionCamera()
{

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, m_aspect, 0.1, 1000.0);

	//postion the camera to look at the origin from far away
	m_camera.m_pos = vector3d(0.0f, 0.0f, 150.0f);
	m_camera.m_view = vector3d(0.0f, 0.0f, 0.0f);
	m_camera.m_up = vector3d(0.0f, 1.0f, 0.0f);   // Y is up 
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
	float change = 1.0f;
	// Check for Ctrl modifier directly
	
	if (m_downKeyState)
	{
		change = -change;
	}

	switch (key) {
	case '1':
		m_camera.m_pos = vector3d(-20.0f, 0.0f, 0.0f);
		m_camera.m_view = vector3d(20.0f, 0.0f, 0.0f);
		m_camera.m_up = vector3d(0.0f, 1.0f, 0.0f);
		break;
	case '2':
		break;
	case 'p':

		if (m_PlanetSpeed + change < 0.0f) // Prevent negative speed
		{
			m_PlanetSpeed = 0.1;
			break;
		}
		m_PlanetSpeed += change;
		break;
	case 's':
		if (m_propellerSpeed + change < 0.0f) // Prevent negative speed
		{
			m_propellerSpeed = 0.1;
			break;

		}
		m_propellerSpeed += change;
		break;
	case 'a':
	{
		if (m_airplaneSpeed + change < 0.0f) // Prevent negative speed
		{
			m_airplaneSpeed = 0.1;
			break;
		}
		m_airplaneSpeed += change;
		break;
	}
	case 'z':
		break;
	case 'r':
		m_PlanetSpeed = 1.0f;
		m_airplaneSpeed = 0.2f;
		m_propellerSpeed = 5.0f;
		break;
	case '+':
		m_camera.m_pos.setZ(m_camera.m_pos.z() + 5.0f);
		break;
	case '_':
		m_camera.m_pos.setZ(m_camera.m_pos.z() - 5.0f);
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
			m_downKeyState = false;
			break;
		case GLUT_KEY_DOWN:
			m_downKeyState = true;
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
	glutKeyboardFunc(KeyboardHandler);       // Register The Keyboard Down Handler
	glutSpecialFunc(SpecialKeyHandler);          // Register Special Keys Handler
	glutMouseFunc(MouseButtonHandler);           // Register Mouse buttons
	glutIdleFunc(NULL);                          // We Do Rendering In Idle Time
	glutMainLoop();                              // Go To GLUT Main Loop
	return 0;
}

/// <summary>
/// This function updates the angles for the moon orbit and propeller rotation based on elapsed time.
/// </summary>
void Update()
{
	double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	static double lastTime = currentTime;
	double deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	m_PlanetAngle = fmod(m_PlanetAngle + m_PlanetSpeed * (float)deltaTime * 60.0f, 360.0f);
	m_AirplaneAngle = fmod( m_AirplaneAngle + m_airplaneSpeed * (float)deltaTime * 60.0f,360.0f);
	m_ProperllerAngle = fmod(m_ProperllerAngle + m_propellerSpeed * (float)deltaTime * 60.0f,360.0f);

}

void RenderScene()
{
	
	Update();
	CreateLightOfSun();
	CreateLightOfAirplane();
	// Draw the scene
	//CreateTheCenterPlanet();
	CreatePlanetUsingTexture();
	//PositionMoon(m_PlanetAngle);
	PositionMoonWithTexture(m_PlanetAngle);
	PositionAirplane(m_AirplaneAngle);
	//PositionSun(m_PlanetAngle);
	PositionSunWithTexture(m_PlanetAngle);
}

// Our Rendering Is Done Here
void Render(void)   
{
	ChangeColourOfBackground();
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