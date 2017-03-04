#pragma once

typedef struct vector2d
{
	float x;
	float y;
} vector2d_t;

vector2d_t vector2d_zero(void);
vector2d_t vector2d(float y, float z);

float vector2d_squared_magnitude(const vector2d_t* vector);
float vector2d_magnitude(const vector2d_t* vector);
float vector2d_normalize(vector2d_t* vector);
vector2d_t vector2d_scaled(const vector2d_t* vector, float factor);
vector2d_t vector2d_negation(const vector2d_t* vector);

float vector2d_dot_product(const vector2d_t* a, const vector2d_t* b);
vector2d_t vector2d_add(const vector2d_t* a, const vector2d_t* b);
vector2d_t vector2d_subtract(const vector2d_t* a, const vector2d_t* b);
