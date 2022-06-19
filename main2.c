/* CLI IRC Client */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include "includes/tools.h"
#include "includes/config.h"
#include "includes/filesystem.h"

/* ALWAYS GLOBAL */
unsigned char number_of_servers = 0;
unsigned char server_chosen = 0;

int main(int argc, char *argv[])
{

	/* define variables */
	struct Server servers[MAX_SERVERS];
	struct Room rooms[MAX_ROOMS];
	unsigned char number_of_servers = 0;

	(void)argc;
	(void)argv;

	number_of_servers = process_server_list_file(&servers);

#if AUTOSTART == true
	if (!number_of_servers) {
		fprintf(stderr,"There are no servers saved.  "
				"Autostart option invalid\n");

	} else {
		server_chosen = 1;
	}
#endif

#if AUTOSTART == false
	if(number_of_servers) {
		/* iterate Server struct and */
		/* let user choose the server */

		print_server_list();
		while(!server_chosen) {
				/*server choice loop */
			printf("Choose the server: \n");

		}
	} 
#endif

	if(!server_chosen) {
		/* prompt user for server addition */
		/* asking for IP address, port, etc */

	}


	return 0;
}
