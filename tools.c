#include "includes/tools.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void fast_strcat(struct String *dest, unsigned char number_of_elements, ...) 
{
	va_list pointer;
	int i;
	struct String *src_ptr;
	va_start(pointer,number_of_elements);
	for (i = 0;i < number_of_elements;i++) {
		src_ptr = va_arg(pointer,struct String *);
		memcpy(dest->text,src_ptr->text,(size_t)src_ptr->size);
		dest->size += src_ptr->size;
	}

	va_end(pointer);

}

