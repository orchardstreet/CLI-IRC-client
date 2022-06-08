#ifndef TOOLS
#define TOOLS
#define true 1
#define false 0

struct String {
	char *text;
	unsigned int size = 0;
};

struct URI {
	struct String ip_address;
	struct String domain;
	unsigned short port;
};

struct Server {
	int sock_fd;
	unsigned char total_rooms;
	unsigned char current_room;
	struct Room *rooms;
	struct String current_room_name;
	struct String name;
	struct URI uri;
	FILE *saved_rooms;
};


void fast_strcat(struct String *dest, unsigned char number_of_elements, ...);

#endif
