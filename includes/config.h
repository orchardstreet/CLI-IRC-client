
#ifndef CONFIG
#define CONFIG
#include "tools.h"

/* USER CAN CHANGE THIS STUFF TO WHATEVER */
#define AUTOSTART false

/* THIS SHIT PROBABLY SHOULDN'T BE CHANGED BY USER */

#define ROOM_NAME_LIMIT 254 /* arbitrary length */
#define HOST_NAME_LIMIT 2048 /* hostname is URL or IP, limited by  max URL bytes */
#define MAX_SERVERS 30
#define MAX_ROOMS 255
#define ROOM_TEXT LIMIT 6000
#define MESSAGE_LIMIT 512
#define INPUT_LIMIT (MESSAGE_LIMIT + 3)
#define NICK_NAME_LIMIT 10
#define BUFFER_LIMIT (((MESSAGE_LIMIT * 2) + (CHANNEL_LIMIT * 4) + (NICK_LIMIT * 4)) * 2)

enum exit_status{SUCCESSC, ERRORC, SYSTEM_ERRORC};

/* enum exit_status SYSTEM_ERRORC ERRORC SUCCESSC */
void exit_program(char *the_error,unsigned char iterate_memory, unsigned char status);

#endif
