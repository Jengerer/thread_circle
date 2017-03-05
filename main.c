#include <time.h>
#include "generation.h"
#include "graphics.h"
#include "shared.h"
#include "vector2d.h"

#define SAMPLE_RADIUS 2
#define LOG_FREQUENCY 1000
#define FITTEST_COUNT 1
#define OFFSPRING_PER_FITTEST 1
#define CANDIDATE_COUNT (FITTEST_COUNT + (FITTEST_COUNT * OFFSPRING_PER_FITTEST))

int main(int argc, char** argv)
{
	const unsigned int seed = (unsigned int)(time(NULL));
	srand(seed);

	graphics_context_t graphics_context = null_graphics_context();
	if (!initialize_graphics(&graphics_context))
	{
		destroy_graphics(&graphics_context);
		pause();
		return -1;
	}

	// Prepare buffer for pixel differences
	const size_t difference_pixel_count = APPLICATION_WIDTH * APPLICATION_HEIGHT;
	const size_t difference_buffer_size = difference_pixel_count * sizeof(GLfloat);
	GLfloat* difference_pixels = (GLfloat*)malloc(difference_buffer_size);

	// Vertices of line points
	vector2d_t line_vertices[POINT_COUNT];
#if USE_CIRCLE
	const float CIRCLE_RADIUS = TEXTURE_HEIGHT * 0.5f;
	const float PI = 3.1415926f;
	const float CENTER_X = TEXTURE_WIDTH / 2.f;
	const float CENTER_Y = TEXTURE_HEIGHT / 2.f;
	for (size_t i = 0; i < POINT_COUNT; ++i)
	{
		const float angle = 2.f * PI * ((float)i / POINT_COUNT);
		const float x = CENTER_X + (CIRCLE_RADIUS * sinf(angle));
		const float y = CENTER_Y + (CIRCLE_RADIUS * cosf(angle));
		line_vertices[i] = vector2d(x, y);
	}
#else
	vector2d_t* current_vertex = line_vertices;
	for (size_t i = 0; i < SQUARE_SIDE_POINTS; ++i)
	{
		const float factor = ((float)i / SQUARE_SIDE_POINTS);
		const float x = TEXTURE_WIDTH * factor;
		const float y = TEXTURE_HEIGHT * factor;
		*current_vertex++ = vector2d(x, 0.f);
		*current_vertex++ = vector2d(x, TEXTURE_HEIGHT);
		*current_vertex++ = vector2d(0.f, y);
		*current_vertex++ = vector2d(TEXTURE_WIDTH, y);
	}
#endif

	GLuint line_vertex_buffer;
	glGenBuffers(1, &line_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

	// Generate index buffer
	GLuint line_index_buffer;
	glGenBuffers(1, &line_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_index_buffer);

	// Candidates with line points
	generation_t candidates[CANDIDATE_COUNT];
	for (size_t i = 0; i < CANDIDATE_COUNT; ++i)
	{
		candidates[i] = create_generation();
	}

	// Create triangle buffer
	const vector2d_t vertices[] =
	{
		vector2d(-1.f, 1.f), vector2d(0.f, 0.f),
		vector2d(1.f, 1.f), vector2d(1.f, 0.f),
		vector2d(1.f, -1.f), vector2d(1.f, 1.f),
		vector2d(-1.f, -1.f), vector2d(0.f, 1.f)
	};
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Create index buffer
	const GLuint indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};
	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// Feed indices
	GLint render_mode = 0;
	size_t generation = 0;
	bool finished = false;
	while (!finished)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				finished = true;
				break;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
					case SDLK_UP:
						++render_mode;
						break;

					case SDLK_DOWN:
						--render_mode;
						break;

					default:
						break;
				}
			}
		}

		for (size_t i = 0; i < CANDIDATE_COUNT; ++i)
		{
			generation_t* candidate = &candidates[i];
			const size_t line_index_count = candidate->index_count;
			const GLuint* line_indices = candidate->indices;
			const GLsizei line_indices_size = line_index_count * sizeof(GLuint);
			glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, line_indices_size, line_indices, GL_DYNAMIC_DRAW);

			// Set parameters
			if (!activate_material(&graphics_context.line_material, LINE_MATERIAL))
			{
				printf("Failed to activate line material.\n");
				destroy_graphics(&graphics_context);
				return -1;
			}
			if (!set_projection(&graphics_context.line_material, TEXTURE_WIDTH, TEXTURE_HEIGHT))
			{
				pause();
				destroy_graphics(&graphics_context);
				return -1;
			}

			// Draw to texture first
			glBindFramebuffer(GL_FRAMEBUFFER, graphics_context.frame_buffer);
			glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT);

			// Draw the quad
			glDrawElements
			(
				GL_LINE_STRIP,
				line_index_count,
				GL_UNSIGNED_INT,
				NULL
			);

			// Set main buffer back
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, APPLICATION_WIDTH, APPLICATION_HEIGHT);

			// Bind vertex and index buffer back
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

			// Now draw that texture on screen
			if (!activate_material(&graphics_context.texture_material, TEXTURE_MATERIAL))
			{
				printf("Failed to activate texture material!\n");
				destroy_graphics(&graphics_context);
				pause();
				return -1;
			}

			if (!set_texture(&graphics_context.texture_material, graphics_context.texture_target, graphics_context.texture_image))
			{
				pause();
				destroy_graphics(&graphics_context);
				return -1;
			}

			// Draw the quad
			if (!set_mode(&graphics_context.texture_material, 0))
			{
				destroy_graphics(&graphics_context);
				pause();
				return -1;
			}

			if (!set_sample_radius(&graphics_context.texture_material, (float)SAMPLE_RADIUS / (float)TEXTURE_WIDTH))
			{
				destroy_graphics(&graphics_context);
				pause();
				return -1;
			}

			GLsizei quad_index_count = sizeof(indices) / sizeof(GLuint);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawElements
			(
				GL_TRIANGLES,
				quad_index_count,
				GL_UNSIGNED_INT,
				NULL
			);

			// Now calculate difference sum
			GLfloat sum = 0.f;
			glReadPixels(0, 0, APPLICATION_WIDTH, APPLICATION_HEIGHT, GL_RED, GL_FLOAT, difference_pixels);
			for (size_t j = 0; j < difference_pixel_count; ++j)
			{
				sum += difference_pixels[j];
			}
			candidate->score = sum;
			printf("%d had a score of %f\n", (int)i, sum);

			// Draw the current best
			// if (i == 1)
			{
				/*set_mode(&graphics_context.texture_material, render_mode);
				glDrawElements
				(
					GL_TRIANGLES,
					quad_index_count,
					GL_UNSIGNED_INT,
					NULL
				);
				*/
				printf("NOW %d\n", (int)i);
				pause();
				SDL_GL_SwapWindow(graphics_context.window);
				pause();
			}
		}

		// Now sort the candidates by score
		qsort(&candidates, CANDIDATE_COUNT, sizeof(generation_t), &compare_generations);

		// Generate off-spring for the best ones
		printf("On generation #%d, the fittest are:\n", (int)++generation);
		generation_t* current_offspring = &candidates[FITTEST_COUNT];
		for (size_t i = 0; i < FITTEST_COUNT; ++i)
		{
			const generation_t* fittest = &candidates[i];
			printf("Fittest: ");
			for (size_t j = 0; j < fittest->index_count; ++j)
			{
				if (j != 0)
				{
					printf(", ");
				}
				printf("%u", fittest->indices[j]);
			}
			printf("\n");

			printf("#%d has score %f with %d lines...\n", (int)i + 1, fittest->score, (int)fittest->index_count);
			for (size_t j = 0; j < OFFSPRING_PER_FITTEST; ++j, ++current_offspring)
			{
				mutate_generation(fittest, current_offspring);
				for (size_t k = 0; k < current_offspring->index_count; ++k)
				{
					if (k != 0)
					{
						printf(", ");
					}
					printf("%u", current_offspring->indices[k]);
				}
			}
			printf("\n");
		}
		printf("\n");
	}
	
	// Shutdown
	destroy_graphics(&graphics_context);
	return 0;
}
