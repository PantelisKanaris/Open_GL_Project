#pragma once
#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>           
#include <math.h>           
#include <windows.h>         
#include <gl/glut.h>       
#include <time.h>
void Render(void);
void InitializeLights(void);
bool init(void);
void Reshape(int w, int h);
void KeyboardDownHandler(unsigned char key, int x, int y);
void SpecialKeyHandler(int a_keys, int x, int y);
void drawAxes();


struct vector3d
{
	float X, Y, Z;

	inline vector3d(void) {}

	// ===== Getters =====
	inline float x() const { return X; }
	inline float y() const { return Y; }
	inline float z() const { return Z; }

	// ===== Setters=====
	inline void setX(float x) { X = x; }
	inline void setY(float y) { Y = y; }
	inline void setZ(float z) { Z = z; }


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


struct CameraPosition
{
	vector3d m_pos;      // Camera position
	vector3d m_view;     // Camera view
	vector3d m_up;       // Camera up vector
	CameraPosition()
	{
		m_pos = vector3d(0.0f, 0.0f, 0.0f);
		m_view = vector3d(0.0f, 0.0f, 0.0f);
		m_up = vector3d(0.0f, 1.0f, 0.0f);
	}
};

#endif // !1
