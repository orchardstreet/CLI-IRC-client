#ifndef TOOLS
#define TOOLS


struct Ip_address_string {
	char text[IP_ADDRESS_LIMIT + 1];
	unsigned int size = 0;
};
struct Host_name_string {
	char text[HOST_NAME_LIMIT + 1];
	unsigned int size = 0;
};
struct Server_name_string {
	char text[SERVER_NAME_LIMIT + 1];
	unsigned int size = 0;
};
struct Room_name_string {
	char text[ROOM_NAME_LIMIT + 1];
	unsigned int size = 0;
};

struct String {
	char *text;
	unsigned int size = 0;
};

struct URI {
	struct Ip_address_string ip_address;
	struct Host_name_string hostname;
	unsigned short port;
};

struct Server {
	int sock_fd;
	unsigned char total_rooms;
	struct Server_name_string name;
	struct URI uri;
	FILE *saved_rooms;
};

struct Room {
	/* for keeping track of how much is in memory,
	 * for example so it gets saved to disk if needed
	 * after full */
	char room_name[ROOM_NAME_LIMIT + 1];
	char recent_text[SCREEN_TEXT_LIMIT * 2];
	char *recent_text_position = NULL;
	char *recent_text_ending = NULL;

	/* for when we want to browse history past what's in memory */
	/* 0 is false, 1 is true */
	unsigned char browse_mode = 0;
	FILE *room_file;
	char browse_file[SCREEN_TEXT_LIMIT * 2];
	char *browse_file = NULL;
	char *browse_file_ending = NULL;

	/* server->sock_fd being -1 means disconnected */
	struct Server *server;

	/* if true, save data to disk,
	 * if false, does not save data to disk */
	/* 0 is false, 1 is true */
	unsigned char save_mode = 0;
}


void fast_strcat(struct String *dest, unsigned char number_of_elements, ...);

#endif
