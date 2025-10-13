#include <iostream>
#include "MyMath.h"
using namespace std;

/// <summary>
/// Linear interpolation function is when i want to map one range of numbers to another range of numbers.
/// To do that i have to know the min and max of the range i want to map to and a number in the range [0,1] that will tell me how far along the range i want to map.
/// The function will return the mapped number.
/// Caviat to map corectly i need to make my number that i want to map from 0-1 (to be used as a percentage) to do that i just divide by the total amount of number
/// </summary>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="number"></param>
/// <returns></returns>
float LinearInterpolation(float min, float max, float number)
{
	return min + number * (max - min);
}
