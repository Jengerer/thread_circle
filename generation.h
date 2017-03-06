#ifndef GENERATION_H
#define GENERATION_H

#include "shared.h"
#if defined(WIN32)
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>

// Structure representing a specific generation of lines
typedef struct generation
{
	GLuint* indices;
	size_t index_count;
	GLfloat* score_buffer;
	GLfloat score;
#if defined(WIN32)
	HANDLE score_thread;
#else
	pthread_t score_thread;
#endif
} generation_t;

generation_t create_generation(void);
void destroy_generation(generation_t* generation);
void mutate_generation(const generation_t* source, generation_t* destination);
void copy_generation(const generation_t* source, generation_t* destination);

int compare_generations(const void* a, const void* b);

// Thread function for computing score
void start_score_thread(generation_t* generation);
void join_score_thread(generation_t* generation);

#if defined(WIN32)
DWORD WINAPI compute_score(LPVOID generation_pointer);
#else
void* compute_score(void* generation_pointer);
#endif

#endif // GENERATION_H
