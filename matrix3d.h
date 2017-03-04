#pragma once

#define MATRIX3D_DIMENSIONS 3

typedef struct matrix3d
{
	float elements[MATRIX3D_DIMENSIONS][MATRIX3D_DIMENSIONS];
} matrix3d_t;
