#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "graphics.h"
#include "shared.h"
#include "vector2d.h"

#define CIRCLE_POINTS 360
#define LINES 2000
#define LINES_INDEX_COUNT (LINES * 2)

int SDL_main(int argc, char** argv)
{
	const unsigned int seed = (unsigned int)(time(NULL));
	srand(seed);

	graphics_context_t graphics_context = null_graphics_context();
	if (!initialize_graphics(&graphics_context))
	{
		system("pause");
		destroy_graphics(&graphics_context);
		return -1;
	}

	// Prepare buffer for pixel differences
	const size_t difference_pixel_count = APPLICATION_WIDTH * APPLICATION_HEIGHT;
	const size_t difference_buffer_size = difference_pixel_count * sizeof(float);
	float* difference_pixels = (float*)malloc(difference_buffer_size);

	// Vertices of line points
	const float CIRCLE_RADIUS = TEXTURE_HEIGHT * 0.5f;
	const float PI = 3.1415926f;
	const float CENTER_X = TEXTURE_WIDTH / 2.f;
	const float CENTER_Y = TEXTURE_HEIGHT / 2.f;
	vector2d_t line_vertices[CIRCLE_POINTS];
	for (size_t i = 0; i < CIRCLE_POINTS; ++i)
	{
		const float angle = 2.f * PI * ((float)i / CIRCLE_POINTS);
		const float x = CENTER_X + (CIRCLE_RADIUS * sinf(angle));
		const float y = CENTER_Y + (CIRCLE_RADIUS * cosf(angle));
		line_vertices[i] = vector2d(x, y);
	}
	GLuint line_vertex_buffer;
	glGenBuffers(1, &line_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

	// Generate index buffer
	GLuint line_index_buffer;
	glGenBuffers(1, &line_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, line_index_buffer);

	// Keep track of best
	float minimum_difference_sum = 10e30f;
	GLuint best_line_indices[LINES_INDEX_COUNT];
	GLsizei best_line_index_count = 2;

	// Indices of line points, start with just a single line
	GLuint line_indices[LINES_INDEX_COUNT];
	line_indices[0] = (GLuint)(rand() % CIRCLE_POINTS);
	line_indices[1] = (GLuint)(rand() % CIRCLE_POINTS);
	GLsizei line_index_count = 2;

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
	bool finished = false;
	while (true)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				finished = true;
				break;
			}
		}

		// Tweak randomly
		const int modification = rand() % 3;
		switch (modification)
		{
			case 0:
			{
				// Randomly change an index
				const size_t random_index = (size_t)(rand() % line_index_count);
				const GLuint new_value = (GLuint)(rand() % CIRCLE_POINTS);
				line_indices[random_index] = new_value;
				break;
			}

			case 1:
			{
				// Add a new index at a random spot
				if (line_index_count < LINES_INDEX_COUNT)
				{
					// Shift all to the right
					GLuint copy_value = (GLuint)(rand() % CIRCLE_POINTS);
					const size_t insert_before = (size_t)(rand() % (line_index_count + 1));
					for (GLsizei i = insert_before; i < line_index_count + 1; ++i)
					{
						GLuint saved = line_indices[i];
						line_indices[i] = copy_value;
						copy_value = saved;
					}

					++line_index_count;
				}
				break;
			}

			case 2:
			{
				// Remove an index at random
				if (line_index_count > 2)
				{
					// Shift all to the left after
					const size_t removed_index = (size_t)(rand() % line_index_count);
					for (GLsizei i = removed_index + 1; i < line_index_count; ++i)
					{
						line_indices[i - 1] = line_indices[i];
					}

					--line_index_count;
				}
				break;
			}
		}

		// Copy new indices
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
			system("pause");
			destroy_graphics(&graphics_context);
			return -1;
		}

		// Draw to texture first
		glBindFramebuffer(GL_FRAMEBUFFER, graphics_context.frame_buffer);
		glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, line_vertex_buffer);
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
		glClear(GL_COLOR_BUFFER_BIT);

		// Bind vertex and index buffer back
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

		// Now draw that texture on screen
		if (!activate_material(&graphics_context.texture_material, TEXTURE_MATERIAL))
		{
			printf("Failed to activate texture material!\n");
			system("pause");
			destroy_graphics(&graphics_context);
			return -1;
		}

		if (!set_texture(&graphics_context.texture_material, graphics_context.texture_target, graphics_context.texture_image))
		{
			system("pause");
			destroy_graphics(&graphics_context);
			return -1;
		}

		// Draw the quad
		set_mode(&graphics_context.texture_material, 0);
		GLsizei index_count = sizeof(indices) / sizeof(GLuint);
		glDrawElements
		(
			GL_TRIANGLES,
			index_count,
			GL_UNSIGNED_INT,
			NULL
		);

		// Now calculate difference sum
		float sum = 0.f;
		glReadPixels(0, 0, APPLICATION_WIDTH, APPLICATION_HEIGHT, GL_RED, GL_FLOAT, difference_pixels);
		for (size_t i = 0; i < difference_pixel_count; ++i)
		{
			sum += difference_pixels[i];
		}

		// Compare and copy if better
		if ((sum < minimum_difference_sum) || ((modification == 2) && (sum == minimum_difference_sum)))
		{
			minimum_difference_sum = sum;
			memcpy(best_line_indices, line_indices, line_indices_size);
			best_line_index_count = line_index_count;

			// Draw the lines
			set_mode(&graphics_context.texture_material, 1);
			glDrawElements
			(
				GL_TRIANGLES,
				index_count,
				GL_UNSIGNED_INT,
				NULL
			);
			SDL_GL_SwapWindow(graphics_context.window);
		}
		else
		{
			// Revert, this one's worse
			memcpy(line_indices, best_line_indices, best_line_index_count * sizeof(GLuint));
			line_index_count = best_line_index_count;
		}
	}
	
	// Shutdown
	system("pause");
	destroy_graphics(&graphics_context);
	return 0;
}