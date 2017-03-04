#pragma once

#if defined(WIN32)
#include <Windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL.h>
#include <stdbool.h>

#include "material.h"

#define RENDER_TARGET_COUNT 1

#define INVALID_TEXTURE 0
#define INVALID_BUFFER 0

typedef struct graphics_context
{
	SDL_Window* window;
	SDL_GLContext gl_context;
	material_t line_material;
	material_t texture_material;

	// Render target
	GLuint frame_buffer;
	GLuint texture_target;
	float* texture_image_buffer;
	GLuint texture_image;
} graphics_context_t;

graphics_context_t null_graphics_context();
bool initialize_graphics(graphics_context_t* out);
void destroy_graphics(graphics_context_t* graphics_context);
