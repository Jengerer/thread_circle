#include "vector2d.h"
#include <math.h>

vector2d_t vector2d_zero(void)
{
	return vector2d(0.f, 0.f);
}

vector2d_t vector2d(float x, float y)
{
	vector2d_t result;
	result.x = x;
	result.y = y;
	return result;
}

float vector2d_squared_magnitude(const vector2d_t* vector)
{
	return vector2d_dot_product(vector, vector);
}

float vector2d_magnitude(const vector2d_t* vector)
{
	const float squared_magnitude = vector2d_squared_magnitude(vector);
	const float magnitude = sqrtf(squared_magnitude);
	return magnitude;
}

float vector2d_normalize(vector2d_t* vector)
{
	const float magnitude = vector2d_magnitude(vector);
	if (magnitude != 0.f)
	{
		const float inverse_magnitude = 1.f / magnitude;
		*vector = vector2d_scaled(vector, inverse_magnitude);
	}
	return magnitude;
}

vector2d_t vector2d_scaled(const vector2d_t* vector, float factor)
{
	const vector2d_t result = vector2d(vector->x * factor, vector->y * factor);
	return result;
}

vector2d_t vector2d_negation(const vector2d_t* vector)
{
	return vector2d_scaled(vector, -1.f);
}

float vector2d_dot_product(const vector2d_t* a, const vector2d_t* b)
{
	return (a->x * b->x) + (a->y * b->y);
}

vector2d_t vector2d_add(const vector2d_t* a, const vector2d_t* b)
{
	const vector2d_t result = vector2d(a->x + b->x, a->y * b->y);
	return result;
}

vector2d_t vector2d_subtract(const vector2d_t* a, const vector2d_t* b)
{
	const vector2d_t negative_b = vector2d_negation(b);
	return vector2d_add(a, &negative_b);
}
