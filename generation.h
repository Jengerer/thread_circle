#ifndef GENERATION_H
#define GENERATION_H

#include "shared.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <pthread.h>

// Structure representing a specific generation of lines
typedef struct generation
{
	GLuint* indices;
	size_t index_count;
	GLfloat* score_buffer;
	GLfloat score;
	pthread_t score_thread;
} generation_t;

generation_t create_generation(void);
void destroy_generation(generation_t* generation);
void mutate_generation(const generation_t* source, generation_t* destination);
void copy_generation(const generation_t* source, generation_t* destination);
int compare_generations(const void* a, const void* b);

// Thread function for computing score
void* compute_score(void* generation_pointer);

#endif // GENERATION_H
