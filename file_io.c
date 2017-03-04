#include "file_io.h"
#include <stdio.h>
#include <stdlib.h>

file_buffer_t null_file_buffer(void)
{
	file_buffer_t result;
	result.data = NULL;
	result.length = 0;
	return result;
}

bool read_file(const char* filename, file_buffer_t* out)
{
	FILE* file;
	const errno_t error = fopen_s(&file, filename, "r");
	if (error)
	{
		printf("Failed to open file %s for read.\n", filename);
		return false;
	}

	// Get file size
	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		printf("Failed to seek to the end of %s.\n", filename);
		return false;
	}
	const size_t length = ftell(file);
	if (length == 0)
	{
		// Nothing to do
		fclose(file);
		return false;
	}

	// Read file in
	rewind(file);
	const size_t buffer_size = length + 1;
	char* buffer = (char*)malloc(buffer_size);
	if (!buffer)
	{
		fclose(file);
		printf("Failed to allocate buffer to read %s.\n", filename);
		return false;
	}
	const size_t element_count = 1;
	const size_t element_size = length;
	const size_t read = fread_s(buffer, buffer_size, element_size, element_count, file);
	fclose(file);
	if (read != element_count)
	{
		free(buffer);
		return false;
	}

	out->data = buffer;
	out->length = length;
	return true;
}

void destroy_file_buffer(file_buffer_t* file_buffer)
{
	void* data = file_buffer->data;
	if (data)
	{
		free(data);
		file_buffer->data = NULL;
	}
	file_buffer->length = 0;
}
