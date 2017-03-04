#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <stdbool.h>

#define INVALID_SHADER 0
#define INVALID_PROGRAM 0
#define INVALID_LOCATION -1

typedef enum material_type
{
	LINE_MATERIAL,
	TEXTURE_MATERIAL
} material_type_t;

typedef struct material
{
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
} material_t;

material_t null_material();
bool create_material
(
	const char* vertex_file,
	const char* fragment_file,
	material_t* out
);
void destroy_material(material_t* material);
bool activate_material(material_t* material, material_type_t type);
bool set_projection(material_t* material, int width, int height);
bool set_texture(material_t* material, GLuint line_texture, GLuint image_texture);
bool set_mode(material_t* material, GLint mode);
