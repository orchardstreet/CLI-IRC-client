#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) 
{
	struct String {
		char *text;
		int limit;
	};

	struct Horse {
		char name_array[50];
		struct String name = {name_array, 90};
		unsigned char house = 0;
	};

	struct Horse little_horse;
	memcpy(little_horse.name_array,"princess",9);
	printf("Horses name: %s\n",little_horse.name.text);

	return 0;
}
