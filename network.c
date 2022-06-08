#include "includes/tools.h"
#include "includes/network.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

unsigned short get_port_from_port_string(struct String *port_string)
{
	/* returns port on success and 0 on failure */
	long port;
	char *strtol_end;

	port = strtol(port_string->text,&strtol_end,10);
	if(port > 65535) {
		fprintf(stderr,"port number cannot exceed 65535"
				"Choose between 1-65535 inclusive\n");
		return 0;
	} else if(*(port_string->text) == '0' && port != 0) {
		fprintf(stderr,"port number cannot be 0\n"
				"Choose between 1-65535 inclusive\n");
		return 0;
	} else if(!port) {
		fprintf(stderr,"error parsing port number\n");
		return 0;
	} else if(port < 0) {
		fprintf(stderr,"port number cannot be negative\n");
		return 0;
	}

	return (unsigned short) port;
}

signed char connect_to_server(int *main_socket,struct String *ip_address, unsigned short port)
{
	/* returns 0 on success and -1 on failure */
	struct sockaddr_in address;
	socklen_t address_len;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip_address->text);
	address.sin_port = htons(port);
	address_len = sizeof(address);

	printf("Connecting to server...\n");
	if ((*main_socket = socket(AF_INET,SOCK_STREAM,0)) == -1) {
		fprintf(stderr,"couldn't create socket\n");
		return -1;
	}
	if((connect(*main_socket,(struct sockaddr *)&address,address_len)) == -1) {
		fprintf(stderr,"couldn't establish connection\n");
		return -1;
	}

	printf("Connected to %s, port %d\n",ip_address->text,port);
	return 0;

}
