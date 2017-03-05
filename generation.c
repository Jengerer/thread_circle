#include "generation.h"
#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Iteration tweaks
enum mutations
{
	CHANGE_INDEX,
	ADD_INDEX,
	REMOVE_INDEX,
	MUTATION_MAX
} mutations_t;

generation_t create_generation(void)
{
	generation_t result;

	// Fill random line
	const size_t buffer_size = LINES_INDEX_COUNT * sizeof(GLuint);
	GLuint* indices = (GLuint*)malloc(buffer_size);
	indices[0] = (GLuint)(rand() % POINT_COUNT);
	indices[1] = (GLuint)(rand() % POINT_COUNT);
	result.indices = indices;
	result.index_count = 2;
	result.score = FLT_MAX;

	return result;
}

void destroy_generation(generation_t* generation)
{
	GLuint* indices = generation->indices;
	if (indices != NULL)
	{
		free(indices);
		generation->indices = NULL;
	}
	generation->index_count = 0;
	generation->score = FLT_MAX;
}

void mutate_generation(const generation_t* source, generation_t* destination)
{
	// Copy over first
	copy_generation(source, destination);
	GLuint* indices = destination->indices;
	const size_t index_count = destination->index_count;

	const int mutation = rand() % MUTATION_MAX;
	switch (mutation)
	{
		case CHANGE_INDEX:
		{
			// Randomly change an index
			const size_t random_index = (size_t)(rand() % index_count);
			const GLuint new_value = (GLuint)(rand() % POINT_COUNT);
			indices[random_index] = new_value;
			assert(indices[random_index] < POINT_COUNT);
			break;
		}

		case ADD_INDEX:
		{
			// Add a new index at a random spot
			if (index_count < LINES_INDEX_COUNT)
			{
				// Shift all to the right
				const GLuint new_index = (GLuint)(rand() % POINT_COUNT);
				const size_t insert_before = (size_t)(rand() % (index_count + 1));
				GLuint copy_value = new_index;
				for (GLsizei i = insert_before; i < index_count + 1; ++i)
				{
					GLuint saved = indices[i];
					indices[i] = copy_value;
					assert(indices[i] < POINT_COUNT);
					copy_value = saved;
				}
				destination->index_count = index_count + 1;
			}
			break;
		}

		case REMOVE_INDEX:
		{
			// Remove an index at random
			if (index_count > 2)
			{
				// Shift all to the left after
				const size_t removed_index = (size_t)(rand() % index_count);
				for (GLsizei i = removed_index + 1; i < index_count; ++i)
				{
					indices[i - 1] = indices[i];
					assert(indices[i - 1] < POINT_COUNT);
				}

				destination->index_count = index_count - 1;
			}
			break;
		}
	}
}

void copy_generation(const generation_t* source, generation_t* destination)
{
	const size_t buffer_size = source->index_count * sizeof(GLuint);
	memcpy(destination->indices, source->indices, buffer_size);
	destination->index_count = source->index_count;
	destination->score = source->score;
}

int compare_generations(const void* a, const void* b)
{
	const generation_t* generation_a = (const generation_t*)a;
	const generation_t* generation_b = (const generation_t*)b;
	const GLfloat score_a = generation_a->score;
	const GLfloat score_b = generation_b->score;
	if (score_a < score_b)
	{
		return -1;
	}
	else if (score_a == score_b)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

