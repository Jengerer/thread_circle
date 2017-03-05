#include <stdio.h>
#include <SDL_image.h>
#include "graphics.h"
#include "shared.h"

#define APPLICATION_TITLE "ThreadCircle"

#define LINE_VERTEX_SHADER "line.vertex"
#define LINE_FRAGMENT_SHADER "line.fragment"
#define TEXTURE_VERTEX_SHADER "texture.vertex"
#define TEXTURE_FRAGMENT_SHADER "texture.fragment"
#define TEXTURE_IMAGE_FILENAME "texture.png"

graphics_context_t null_graphics_context()
{
	graphics_context_t result;
	result.window = NULL;
	result.gl_context = NULL;
	result.line_material = null_material();
	result.texture_material = null_material();
	result.frame_buffer = 0;
	result.texture_target = 0;
	result.texture_image_buffer = NULL;
	result.texture_image = 0;
	return result;
}

bool load_texture_image(graphics_context_t* context)
{
	SDL_Surface* surface = IMG_Load(TEXTURE_IMAGE_FILENAME);
	if (surface == NULL)
	{
		printf("Failed to load texture image from file.\n");
		return false;
	}

	const GLsizei width = surface->w;
	const GLsizei height = surface->h;
	const size_t texture_pixel_count = (size_t)(width * height);
	const size_t texture_buffer_size = texture_pixel_count * sizeof(float);
	float* texture_image_buffer = (float*)malloc(texture_buffer_size);
	float* current_texture_pixel = texture_image_buffer;

	// Average the bytes for the image
	const float maximum_value = 255.f;
	const SDL_PixelFormat* format = surface->format;
	const size_t bytes_per_pixel = (size_t)format->BytesPerPixel;
	const float average_denominator = (float)bytes_per_pixel;
	const uint8_t* pixels = (const uint8_t*)surface->pixels;
	const uint8_t* current = pixels;
	for (size_t i = 0; i < texture_pixel_count; ++i)
	{
		float sum = 0.f;
		for (size_t j = 0; j < bytes_per_pixel; ++j, ++current)
		{
			sum += ((float)*current) / maximum_value;
		}

		*current_texture_pixel++ = sum / average_denominator;
	}

	context->texture_image_buffer = texture_image_buffer;

	// Create texture from buffer
	GLuint texture_image;
	glGenTextures(1, &texture_image);
	glBindTexture(GL_TEXTURE_2D, texture_image);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_LUMINANCE, GL_FLOAT, texture_image_buffer);
	context->texture_image = texture_image;

	SDL_FreeSurface(surface);
	return true;
}

bool initialize_graphics(graphics_context_t* out)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Failed to initialize SDL.\n");
		return false;
	}

	SDL_Window* window = SDL_CreateWindow(
		APPLICATION_TITLE,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		APPLICATION_WIDTH,
		APPLICATION_HEIGHT,
		SDL_WINDOW_OPENGL
	);
	if (!window)
	{
		printf("Failed to initialize window.\n");
		return false;
	}
	out->window = window;

	// Create context
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	if (!gl_context)
	{
		printf("Failed to create OpenGL context.\n");
		return false;
	}
	out->gl_context = gl_context;

	// No deprecated features
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// OpenGL 3.2
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Double-buffer
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Get functions
	glewExperimental = GL_TRUE;
	glewInit();

	// Clear colour and bits
	glClearColor(1.f, 1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.f);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load the target texture file
	if (!load_texture_image(out))
	{
		printf("Failed to load texture image!\n");
		return false;
	}

	// Create the shaders
	if (!create_material(LINE_VERTEX_SHADER, LINE_FRAGMENT_SHADER, &out->line_material))
	{
		printf("Failed to create line material.\n");
		return false;
	}
	else if (!create_material(TEXTURE_VERTEX_SHADER, TEXTURE_FRAGMENT_SHADER, &out->texture_material))
	{
		printf("Failed to create texture material.\n");
		return false;
	}

	// Create texture
	GLuint texture_target;
	const GLint detail_level = 0;
	glGenTextures(RENDER_TARGET_COUNT, &texture_target);
	glBindTexture(GL_TEXTURE_2D, texture_target);
	glTexImage2D(GL_TEXTURE_2D, detail_level, GL_LUMINANCE, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	const GLenum createTextureError = glGetError();
	if (createTextureError != GL_NO_ERROR)
	{
		printf("Failed to create and fill texture.\n");
		return false;
	}
	out->texture_target = texture_target;
	glBindTexture(GL_TEXTURE_2D, 0);

	// Create render target
	GLuint frame_buffer = 0;
	glGenFramebuffers(RENDER_TARGET_COUNT, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	out->frame_buffer = frame_buffer;

	// Bind texture to frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_target, detail_level);
	const GLenum frameBufferTextureError = glGetError();
	if (frameBufferTextureError != GL_NO_ERROR)
	{
		printf("Failed to bind texture to framebuffer: %d (0x%x)\n", frameBufferTextureError, frameBufferTextureError);
		return false;
	}

	GLenum drawBuffers = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(RENDER_TARGET_COUNT, &drawBuffers);
	glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);

	// Check that all's good
	const GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (frameBufferStatus)
		{
			case GL_FRAMEBUFFER_UNDEFINED:
				printf("Framebuffer undefined!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				printf("Framebuffer incomplete attachment!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				printf("Framebuffer incomplete missing attachment!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				printf("Framebuffer incomplete draw buffer!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				printf("Framebuffer incomplete read buffer!\n");
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				printf("Framebuffer unsupported!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				printf("Framebuffer incomplete multisample!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				printf("Framebuffer incomplete layer targets!\n");
				break;

			default:
				printf("Framebuffer error unknown!\n");
				break;
		}

		return false;
	}

	return true;
}

void destroy_graphics(graphics_context_t* graphics_context)
{
	GLuint texture_image = graphics_context->texture_image;
	if (texture_image != INVALID_TEXTURE)
	{
		glDeleteTextures(RENDER_TARGET_COUNT, &texture_image);
		graphics_context->texture_image = INVALID_TEXTURE;
	}

	float* texture_image_buffer = graphics_context->texture_image_buffer;
	if (texture_image_buffer != NULL)
	{
		free(texture_image_buffer);
		graphics_context->texture_image_buffer = NULL;
	}

	GLuint texture_target = graphics_context->texture_target;
	if (texture_target != INVALID_TEXTURE)
	{
		glDeleteTextures(RENDER_TARGET_COUNT, &texture_target);
		graphics_context->texture_target = INVALID_TEXTURE;
	}

	GLuint frame_buffer = graphics_context->frame_buffer;
	if (frame_buffer != INVALID_BUFFER)
	{
		glDeleteFramebuffers(RENDER_TARGET_COUNT, &frame_buffer);
		graphics_context->frame_buffer = INVALID_BUFFER;
	}

	destroy_material(&graphics_context->line_material);
	destroy_material(&graphics_context->texture_material);

	SDL_GLContext* gl_context = graphics_context->gl_context;
	if (gl_context)
	{
		SDL_GL_DeleteContext(gl_context);
		graphics_context->gl_context = NULL;
	}

	SDL_Window* window = graphics_context->window;
	if (window)
	{
		SDL_DestroyWindow(window);
		graphics_context->window = NULL;
	}
}
