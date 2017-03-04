#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct file_buffer
{
	void* data;
	size_t length;
} file_buffer_t;

file_buffer_t null_file_buffer(void);
bool read_file(const char* filename, file_buffer_t* out);
void destroy_file_buffer(file_buffer_t* file_buffer);
