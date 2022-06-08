
#include <stdlib.h>
#include <stdio.h>
#include "includes/config.h"

void exit_program(char *the_error, unsigned char iterate_memory, unsigned char status)
{
	/* free memory */

	if(status == SUCCESSC) {
		exit(EXIT_SUCCESS);
	} else if(status == SYSTEM_ERRORC) {
		perror("");
	}
	fprintf(stderr,"%s\n",the_error);
	exit(EXIT_FAILURE);
}
