#include <stdlib.h>
#include <stdio.h>
#include "includes/tools.h"
#include "includes/config.h"
#include "includes/filesystem.h"

	/*TODO get rid of default values */
signed char validate_number(char **number_string) {
	/* function returns -1 on failure, number on success */
	/* should check that number is 1 - 255 */
	unsigned char number = 1; return number; /* success */
}
unsigned char validate_hostname(char **hostname) {
	/* function returns -1 on failure, 0 on success */
	hostname = "127.0.0.1";

	return 0; /* success */
}
unsigned short validate_port(char **port_string) {
	/* functions return 0 on failure, port on success */
	unsigned short port = 6667;

	return port; /* success */
}
unsigned char validate_servername(char **servername) {
	/* functions return -1 on failure, 0 on success */
	servername = "test server";

	return 0; /* success */
}

unsigned char process_server_list_file(struct Server *servers)
{
	/* returns number of properly formatted servers found and processed into server struct 
	 * and returns 0 on no servers found */
	FILE *servers_list;
	int previous_byte, byte, lines = 0;
	char **server_lines;
	int line_counter;
	size_t len = 0;
	char *line_holder;
	unsigned char number_of_servers = 0;
	unsigned char number;
	char *hostname;
	unsigned short port;
	char *servername;
	char *browse_line;

	/* open servers list file and store in Server struct */
	if((servers_list = fopen("storage/servers_list","r+")) == NULL) {
		perror("");
		fprintf(stderr,"Error trying to open server_list file\n"
				"please make sure it exists in the\n"
				"storage folder");
		return 0;
	}
	
	byte = fgetc(servers_list);
	if (byte == '\n' || byte == EOF) {
		fprintf(stderr,"nothing in servers list file\n");
		return 0;  
	}
	/*TODO make sure program never stores space as first byte of hostname */
	if (byte < '0' || byte > '9') {
		fprintf(stderr,"corrupted first byte of server list file\n"
				"no servers read from file\n");
		return 0;  
	}
	rewind(servers_list);
	do
	{
		previous_byte = byte;
		byte = fgetc(servers_list);
		if(byte != '\n' && byte != EOF && byte < 32 && byte > 126) {
			fprintf(stderr,"no special characters allowed in server list file\n");
			if(!lines) {
				fprintf(stderr,"no servers read from file\n");
				return 0;
			}
		}
		if(byte == '\n' && previous_byte != '\n')
			lines++;
		if(lines > MAX_SERVERS) {
			fprintf(stderr,"Too many servers in server list file, only using first %s servers in file, which is the server number limit currently configured.  To change this configuration, edit config.h\n",MAX_SERVERS);
			break;
		}
	} while (byte != EOF);
	rewind(servers_list);
	server_lines = malloc(sizeof(server_lines) * lines);
	for(line_counter = 0; line_counter < lines; line_counter++) {
		server_lines[line_counter] = NULL;
		len = 0;
		/*TODO change first parameter of this to Server struct */
		/*TODO make sure these functions like validatehostname check for
		 * first null byte */
		getline(&line_holder,&len,servers_list);
		browse_line = line_holder;

		if((number = validate_number(&browse_line)) == -1) {
			fprintf(stderr,"line %d in server file should start with a server number < MAX_SERVERS as defined in config.h, skipping processing this server\n",line_counter);
			return 0;
		}
		line_holder++;
		if(line_holder != ' ') {
			fprintf(stderr,"line %d in server file has a missing space, skipping processing this server\n",line_counter);
			continue;
		}
		if(validate_hostname(&browse_line) == -1) {
			fprintf(stderr,"line %d in server file has an invalid hostname, skipping processing this server\n",line_counter);
			continue;
		}
		line_holder++;
		if(line_holder != ' ') {
			fprintf(stderr,"line %d in server file has a missing space, skipping processing this server\n",line_counter);
			continue;
		}
		hostname = browse_line;

		if(!(port = validate_port(&browse_line))) {
			fprintf(stderr,"line %d in server file has an invalid port, skipping processing this server\n",line_counter);
			continue;
		}
		line_holder++;
		if(line_holder != ' ') {
			fprintf(stderr,"line %d in server file has a missing space, skipping processing this server\n",line_counter);
			continue;
		}
	

		if(validate_servername(&browse_line) == -1) {
			fprintf(stderr,"line %d in server file has an invalid server name, skipping processing this server\n",line_counter);
			continue;
		}
		line_holder++;
		if(*browse_line != '\n' || *browse_line != EOF) {
			fprintf(stderr,"line %d in server file has junk at end of line, skipping processing this server\n",line_counter);
			continue;
		}
		servername = browse_line;
		/* line in server list file is valid, add to struct */
		*numberofservers = *numberofservers + 1;
	}
	fclose(servers_list);


	return number_of_servers;
}
