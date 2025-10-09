#pragma once
#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>           
#include <math.h>           
#include <windows.h>         
#include <gl/glut.h>       
#include <time.h>
void render(void);
void initLights(void);
bool init(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void special_keys(int a_keys, int x, int y);
void drawAxes();


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

#endif // !1
