#ifndef GENERATION_H
#define GENERATION_H

#include "shared.h"
#include <GL/glew.h>
#include <GL/gl.h>

// Structure representing a specific generation of lines
typedef struct generation
{
	GLuint* indices;
	size_t index_count;
	GLfloat score;
} generation_t;

generation_t create_generation(void);
void destroy_generation(generation_t* generation);
void mutate_generation(const generation_t* source, generation_t* destination);
void copy_generation(const generation_t* source, generation_t* destination);
int compare_generations(const void* a, const void* b);

#endif // GENERATION_H
