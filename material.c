#include "file_io.h"
#include "material.h"
#include "matrix3d.h"
#include "vector2d.h"

#include <stdio.h>
#include <stdlib.h>

#define POSITION_ATTRIBUTE_NAME "in_position"
#define UV_ATTRIBUTE_NAME "in_uv"
#define PROJECTION_MATRIX_NAME "projection"
#define LINE_TEXTURE_SAMPLER_NAME "line_texture"
#define IMAGE_TEXTURE_SAMPLER_NAME "image_texture"
#define MODE_NAME "mode"

material_t null_material()
{
	material_t material;
	material.vertex_shader = INVALID_SHADER;
	material.fragment_shader = INVALID_SHADER;
	material.program = INVALID_PROGRAM;
	return material;
}

GLuint create_shader(const char* filename, GLenum type)
{
	// Read source
	file_buffer_t source = null_file_buffer();
	if (!read_file(filename, &source))
	{
		printf("Failed to read shader source for %s.\n", filename);
		return INVALID_SHADER;
	}

	GLuint shader = glCreateShader(type);
	if (shader == INVALID_SHADER)
	{
		printf("Failed to create shader object for %s.\n", filename);
		return INVALID_SHADER;
	}
	const size_t source_count = 1;
	const GLchar* source_text = (GLchar*)source.data;
	const GLint source_length = (GLint)source.length;
	glShaderSource(shader, source_count, &source_text, &source_length);
	glCompileShader(shader);

	// Check compliation status
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint error_length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);
		if (error_length > 1)
		{
			GLchar* error_string = (GLchar*)malloc(error_length);
			glGetShaderInfoLog(shader, error_length, &error_length, error_string);
			printf("Shader compilation failed:\n%s\n", error_string);
			free(error_string);
		}
		glDeleteShader(shader);
		return INVALID_SHADER;
	}

	return shader;
}

bool create_program(material_t* out)
{
	GLuint program = glCreateProgram();
	if (program == INVALID_PROGRAM)
	{
		destroy_material(out);
		printf("Failed to create shader program object.\n");
		return false;
	}
	glAttachShader(program, out->vertex_shader);
	glAttachShader(program, out->fragment_shader);
	glLinkProgram(program);

	// Check link status
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint error_length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &error_length);
		if (error_length > 1)
		{
			GLchar* error_string = (GLchar*)malloc(error_length);
			glGetProgramInfoLog(program, error_length, &error_length, error_string);
			printf("Shader compilation failed:\n%s\n", error_string);
			free(error_string);
		}
		glDeleteProgram(program);
		return false;
	}

	out->program = program;
	return true;
}

bool set_line_shader_parameters(material_t* material)
{
	const GLuint program = material->program;
	const GLint position_index = glGetAttribLocation(program, POSITION_ATTRIBUTE_NAME);
	if (position_index == INVALID_LOCATION)
	{
		printf("Failed to find attribute '%s' in shader.\n", POSITION_ATTRIBUTE_NAME);
		return false;
	}

	const GLsizei position_size = sizeof(vector2d_t);
	const GLsizei vertex_size = position_size;
	const GLint position_floats = (GLint)(vertex_size / sizeof(float));
	glEnableVertexAttribArray(position_index);
	glVertexAttribPointer(position_index, position_floats, GL_FLOAT, GL_FALSE, vertex_size, NULL);
	return true;
}

bool set_texture_shader_parameters(material_t* material)
{
	const GLuint program = material->program;
	const GLint position_index = glGetAttribLocation(program, POSITION_ATTRIBUTE_NAME);
	if (position_index == INVALID_LOCATION)
	{
		printf("Failed to find attribute '%s' in shader.\n", POSITION_ATTRIBUTE_NAME);
		return false;
	}

	const GLint uv_index = glGetAttribLocation(program, UV_ATTRIBUTE_NAME);
	if (uv_index == INVALID_LOCATION)
	{
		printf("Failed to find attribute '%s' in shader.\n", UV_ATTRIBUTE_NAME);
		return false;
	}

	const GLsizei position_size = sizeof(vector2d_t);
	const size_t position_offset = 0;
	const GLsizei uv_size = sizeof(vector2d_t);
	const size_t uv_offset = (size_t)position_size;
	const GLsizei vertex_size = position_size + uv_size;
	const GLint position_floats = (GLint)(position_size / sizeof(float));
	const GLint uv_floats = (GLint)(position_size / sizeof(float));
	glEnableVertexAttribArray(position_index);
	glEnableVertexAttribArray(uv_index);
	glVertexAttribPointer(position_index, position_floats, GL_FLOAT, GL_FALSE, vertex_size, (const void*)position_offset);
	glVertexAttribPointer(uv_index, uv_floats, GL_FLOAT, GL_FALSE, vertex_size, (const void*)uv_offset);
	return true;
}

bool create_material
(
	const char* vertex_file,
	const char* fragment_file,
	material_t* out
)
{
	GLuint vertex_shader = create_shader(vertex_file, GL_VERTEX_SHADER);
	if (vertex_shader == INVALID_SHADER)
	{
		printf("Failed to load vertex shader from %s.\n", vertex_file);
		return false;
	}
	out->vertex_shader = vertex_shader;

	GLuint fragment_shader = create_shader(fragment_file, GL_FRAGMENT_SHADER);
	if (fragment_shader == INVALID_SHADER)
	{
		destroy_material(out);
		printf("Failed to load fragment shader from %s.\n", fragment_file);
		return false;
	}
	out->fragment_shader = fragment_shader;

	// Link the shader
	if (!create_program(out))
	{
		destroy_material(out);
		printf("Failed to create shader program.\n");
		return false;
	}

	return true;
}

void destroy_material(material_t* material)
{
	const GLuint program = material->program;
	if (program != INVALID_PROGRAM)
	{
		glDeleteProgram(program);
		material->program = INVALID_PROGRAM;
	}

	const GLuint vertex_shader = material->vertex_shader;
	if (vertex_shader != INVALID_SHADER)
	{
		glDeleteShader(vertex_shader);
		material->vertex_shader = INVALID_SHADER;
	}
	
	const GLuint fragment_shader = material->fragment_shader;
	if (fragment_shader != INVALID_SHADER)
	{
		glDeleteShader(fragment_shader);
		material->fragment_shader = INVALID_SHADER;
	}
}

bool activate_material(material_t* material, material_type_t type)
{
	glUseProgram(material->program);

	// Vertex attributes and uniforms
	switch (type)
	{
	case LINE_MATERIAL:
		if (!set_line_shader_parameters(material))
		{
			printf("Failed to set line shader parameters.\n");
			return false;
		}
		break;

	case TEXTURE_MATERIAL:
		if (!set_texture_shader_parameters(material))
		{
			printf("Failed to set texture shader parameters.\n");
			return false;
		}
		break;

	default:
		printf("Invalid material type specified!\n");
		return false;
	}

	return true;
}

bool set_projection(material_t* material, int width, int height)
{
	GLint location = glGetUniformLocation(material->program, PROJECTION_MATRIX_NAME);
	if (location == INVALID_LOCATION)
	{
		printf("Failed to find uniform '%s'.\n", PROJECTION_MATRIX_NAME);
		return false;
	}

	// Scale X from [0, Width] and Y from [0, Height] to [0, 2], then subtract 1
	// from X and add 1 to Y to get them between [-1, 1].
	matrix3d_t projection;
	float (*elements)[MATRIX3D_DIMENSIONS] = projection.elements;
	elements[0][0] = 2.f / ((float)width);
	elements[0][1] = 0.f;
	elements[0][2] = 0.f;
	elements[1][0] = 0.f;
	elements[1][1] = -2.f / ((float)height);
	elements[1][2] = 0.f;
	elements[2][0] = -1.f;
	elements[2][1] = 1.f;
	elements[2][2] = 1.f;
	const GLsizei matrix_count = 1;
	const GLboolean transpose = GL_FALSE;
	glUniformMatrix3fv(location, matrix_count, transpose, *elements);
	return true;
}

bool set_texture(material_t* material, GLuint line_texture, GLuint image_texture)
{
	const GLint line_location = glGetUniformLocation(material->program, LINE_TEXTURE_SAMPLER_NAME);
	if (line_location == INVALID_LOCATION)
	{
		printf("Failed to find uniform '%s'.\n", LINE_TEXTURE_SAMPLER_NAME);
		return false;
	}

	const GLint image_location = glGetUniformLocation(material->program, IMAGE_TEXTURE_SAMPLER_NAME);
	if (image_location == INVALID_LOCATION)
	{
		printf("Failed to find uniform '%s'.\n", IMAGE_TEXTURE_SAMPLER_NAME);
		return false;
	}

	// Bind line texture to index 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, line_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glUniform1i(line_location, 0);

	// Bind image texture to index 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, image_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glUniform1i(image_location, 1);

	return true;
}

bool set_mode(material_t* material, GLint mode)
{
	const GLint mode_location = glGetUniformLocation(material->program, MODE_NAME);
	if (mode_location == INVALID_LOCATION)
	{
		printf("Failed to find uniform '%s'.\n", MODE_NAME);
		return false;
	}

	glUniform1i(mode_location, mode);
	return true;
}
