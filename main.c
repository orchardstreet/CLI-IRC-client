/* relevant rfcs: 
 * rfc1459 Internet Relay Chat Protocol, published in 1993  https://datatracker.ietf.org/doc/html/rfc1459
 * rfc2812 Internet Relay Chat: Client Protocol, published in 2000 https://datatracker.ietf.org/doc/html/rfc2812
 * proposal ref 2022 https://modern.ircdocs.horse */
/* TODO respond with PONG */
/* TODO create recv buffer of 4000 */
/* TODO create output buffer of 600 */
/* CLI IRC Client */
/* William Lupinacci <will.lupinacci@gmail.com>
 * All Rights Reserved © 2022 */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

/* 255 ascii characters, not including the ending '\0' */
#define CHANNEL_LIMIT 254
#define HOSTNAME_LIMIT 39
#define SERVER_NAME_LIMIT 254
#define MAX_SERVERS 30
#define MAX_ROOMS 255
#define ROOM_TEXT_LIMIT 6000
#define MALLOCD_MEMORY_ESTIMATE (MAX_SERVERS * (SERVER_NAME_LIMIT + HOSTNAME_LIMIT + 2)) + (MAX_SERVERS * MAX_ROOMS * (ROOM_TEXT_LIMIT + CHANNEL_LIMIT + 2))

/* 512 ascii characters, not including the ending \r\n\0 */
#define MESSAGE_LIMIT 512

/* The three extra bytes includes an extra byte
 * to check for if the message is too large
 * plus the \n\0 fgets adds */
#define INPUT_LIMIT (MESSAGE_LIMIT + 3) 

/* 10 ascii characters , not including the ending '\0' */
#define NICK_LIMIT 10

/* Memory section used for containing the full bytes to send to server, plus for receiving
 * bytes from the server.  Non-user defined bytes to send to the server are never longer 
 * than 50 bytes, plus '\r\n'.  When adding user defined fields to send to the server, check them 
 * individually for overflow in the code and include them in this define to adjust the BUFFER_LIMIT 
 * appropriately.  As it is now this should never have buffer overflows. */
#define BUFFER_LIMIT (((MESSAGE_LIMIT * 2) + (CHANNEL_LIMIT * 4) + (NICK_LIMIT * 4)) * 2)   /* should always be at least 3 * INPUT_LIMIT */ 


unsigned char fast_strcat(char *dest, unsigned char *amount_array, unsigned char number_of_elements,...)
{
	/* Concatenates the strings in va_arg to dest.
	 * Returns the total length of final string minus ending \0 byte,
	 * which is appended automatically for safety  */

	va_list ptr;
	unsigned char i;
	unsigned char amount;
	unsigned char bytes_copied = 0;
	va_start(ptr,number_of_elements);

	for(i = 0; i < number_of_elements; i++) {
		amount = *(amount_array + i);
		bytes_copied += amount;
		memcpy(dest,va_arg(ptr,char *),(size_t)amount);
		dest = dest + amount;
	}

	/* Terminate string with null byte */
	*dest = 0;

	va_end(ptr);

	return bytes_copied;
}

void exit_error(char *the_error)
{
	fprintf(stderr,"%s\n",the_error);
	exit(EXIT_FAILURE);	
}

void exit_system_error(char *the_error)
{
	perror("");
	fprintf(stderr,"%s\n",the_error);
	exit(EXIT_FAILURE);	
}

unsigned char parse_input_and_send_to_server(char *input_browse,char *buf_browse,size_t input_length,int main_socket,char *nick,char *channel,unsigned char *channel_length) {
	/* return 0 on success, and 1 on quit */
	
	unsigned char amount_array[10];
	unsigned char bytes_copied = 0;
	char *buffer = buf_browse;
	char *input = input_browse;
	char *extra_pointer;
	char *sort_pointer;
	ssize_t function_response;
	size_t message_length;
	size_t channel_length_count = 0;

	if(input_length > MESSAGE_LIMIT) {
		fprintf(stderr,"Message must be under 512 bytes\n");
		return 0;
	}

	if(input_length < 1) 
		return 0;

	if(*input_browse != '/') {
		if(*channel == 0) {
			fprintf(stderr,"No channel selected\nSyntax: /join #channel\n");
			return 0;
		}
		amount_array[0] = 8;
		amount_array[1] = *channel_length;
		amount_array[2] = 2;
		bytes_copied = fast_strcat(buffer,amount_array,3,"PRIVMSG ",channel," :");
		buf_browse = buf_browse + bytes_copied;
		printf("%s: %s\n",nick,input);
		message_length = (buf_browse - buffer) + input_length + 2;  
		memcpy(buf_browse,input_browse,input_length);
		buf_browse = buf_browse + input_length;
	} else {	
		input_browse++;
		switch(*input_browse) {
			case 0:
				fprintf(stderr,"Error: no command present\n");
				return 0; 
			case 'j':
				input_browse++;
				if(strstr(input_browse,"oin ") == input_browse) {
					input_browse += 3;
				} else if(*input_browse != ' ') {
					fprintf(stderr,"Error: Wrong syntax\nSyntax: /j #room\n");
					return 0;
				}
				for(;*input_browse == ' ';input_browse++) { } /* skip spaces */
				/* make sure join request has a proper channel name format */
				for(;*input_browse == ',';input_browse++) { } /* skip commas weechat does this*/
				if(*input_browse != '#') {
					fprintf(stderr,"Error: Wrong syntax\nSyntax: /j #room\n");
					return 0; 
				}
				extra_pointer = input_browse + 1;
				/* will only end this for loop
				 * on a null character or a space */
				for(;*extra_pointer && *extra_pointer != ' '; extra_pointer++) {
					if(*extra_pointer < 33 || *extra_pointer > 126)  {
						fprintf(stderr,"Error: No control characters in channel names\n");
						return 0; 

					}
					while(*extra_pointer == ',' && *(extra_pointer + 1) == ',') {
						sort_pointer = extra_pointer;
						do {
							sort_pointer++;
							*(sort_pointer - 1) = *sort_pointer;
						} while(*sort_pointer);
					}
					if(*extra_pointer == ',' && (*(extra_pointer + 1) == ' ' || *(extra_pointer + 1) == '\0')) {
						extra_pointer++;
						break;
					} else if (*extra_pointer == ',' && *(extra_pointer + 1) != '#') {
						break;
					}
					channel_length_count++;
				}
				for(;*extra_pointer == ' ';extra_pointer++) {}
				if (channel_length_count + 1 > CHANNEL_LIMIT) {
					fprintf(stderr,"Error: Channel name/list too long, "
							"must be under 255 characters\n");
					return 0; 
				}
				if(*extra_pointer != '\0' && *extra_pointer != ',') {
					fprintf(stderr,"Error: Channel name or list shouldn't have spaces\n"); return 0; 
				}
				/* copy channel name from input to channel variable */
				*channel_length = channel_length_count + 1;
#ifdef DEBUG
				printf("\nChannel length: %d bytes\n",*channel_length);
#endif
				memcpy(channel,input_browse,*channel_length);
				/* null terminate channel */
				channel[*channel_length] = 0;
				memcpy(buf_browse,"JOIN ",5);
				buf_browse += 5;
				memcpy(buf_browse,input_browse,*channel_length);
				buf_browse += *channel_length;
				message_length = (buf_browse - buffer) + 2;
				break;
			case 'q':
				input_browse++;
				if(*input_browse != 0 && *input_browse != ' ' && (strstr(input_browse,"uit") != input_browse)) { 
					fprintf(stderr,"Error: Not a valid command\n");
					return 0;
				}
				return 1;
			default:
				fprintf(stderr,"Not a command\n");
				return 0;
		}

	}

	/* we get the message length in a prior logic piece */
	/* TODO store buffer and input length in structs, so length don't need  separate variables
	 * this can make it so this long function can be split up */
	memcpy(buf_browse,"\r\n",2);
	buffer[message_length] = 0;
#ifdef DEBUG
	printf("\nmessage length: %lu bytes\n",(unsigned long)message_length);
	printf("raw message out: %s",buffer);
#endif
	/* write message to socket */
	function_response = write(main_socket,buffer,message_length);
	if(function_response == -1)
		exit_system_error("write failed");
	if(function_response == -1)
		exit_system_error("write failed");
	return 0;
}


unsigned short get_port_from_port_string(char *port_string)
{
	/* returns port on success and 0 on failure */
	long port;
	char *strtol_end;

	port = strtol(port_string,&strtol_end,10);
	if(port > 65535) {
		fprintf(stderr,"port number cannot exceed 65535"
				"Choose between 1-65535 inclusive\n");
		return 0;
	} else if(*port_string == '0' && port != 0) {
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

signed char connect_to_server(int *main_socket,char *ip_address, unsigned short port)
{
	/* returns 0 on success and -1 on failure */
	struct sockaddr_in address;
	socklen_t address_len;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip_address);
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


	printf("Connected to %s, port %d\n",ip_address,port);
	return 0;

}

int main(int argc, char *argv[])
{
	
	struct Room {
		char *name;
		char *text;
	};
	struct Server {
		int socket_fd;
		char *hostname;
		char *name;
		unsigned short port;
		unsigned char number_of_rooms_used;
		struct Room *rooms;
		unsigned char active_room;
		char *nick;
	};

	/* define variables */
	char buffer[BUFFER_LIMIT];
	char input[INPUT_LIMIT];
	int main_socket;
	int function_response;
	char channel[CHANNEL_LIMIT + 1];
	unsigned char channel_length;
	char *ip_address;
	char *port_string;
	unsigned char amount_array[10];
	unsigned char bytes_copied;
	char *nick;
	size_t nick_length;
	size_t ip_address_length;
	size_t input_length;
	unsigned short port;
	fd_set set, set_backup;
	double prior_time = time(NULL);
	double now_time;
	
	/* Initialise memory */
	struct Server servers = malloc(sizeof(Server) * MAX_SERVERS);
	for(i = 0; i < MAX_SERVERS; i++) {
		server[i].rooms = malloc(sizeof(Room) * MAX_ROOMS);
		server[i].nick = malloc(NICK_LIMIT + 1);
		server[i].name = malloc(SERVER_NAME_LIMIT + 1);
		server[i].hostname = malloc(HOSTNAME_LIMIT + 1);
		/*fill first byte with 0*/
		server[i].nick[0] = 0;
		server[i].name[0] = 0;
		server[i].hostname[0] = 0;
		for(p = 0; p < MAX_ROOMS; p++) {
			server[i].rooms[p].text = malloc(ROOM_TEXT_LIMIT + 1);
			server[i].rooms[p].name = malloc(CHANNEL_LIMIT + 1);
			/*fill first byte with 0*/
			server[i].rooms[p].text[0] = 0;
			server[i].rooms[p].name[0] = 0; 
		}
	}

	if(argc < 4)
		exit_error("Wrong syntax\nSyntax: ./main "
				"IP_ADDRESS PORT NICK");

	ip_address = argv[1];
	port_string = argv[2];
	nick = argv[3];
	channel[0] = 0;

	if((nick_length = strlen(nick)) >= NICK_LIMIT)
		exit_error("nick must be less than 10 characters");

	if((ip_address_length = strlen(ip_address)) > HOSTNAME_LIMIT)
		exit_error("IP address must be less than 40 character");

	if(!(port = get_port_from_port_string(port_string)))
		exit_error("error getting port number");

	if(connect_to_server(&main_socket,ip_address,port) == -1)
		exit_error("error connecting to server");

	/* Send first message to IRC server */
	/* this can be sped up but not now, by creating separate template buffers at start of program */
	amount_array[0] = 5;
	amount_array[1] = (unsigned char) nick_length;
	amount_array[2] = 2;
	bytes_copied = fast_strcat(buffer,amount_array,3,"NICK ",nick,"\r\n");
	printf("Wrote following to socket: %s",buffer);
	function_response = write(main_socket,buffer,bytes_copied);
	if(function_response == -1)
		exit_system_error("error writing to socket");

	/* Send second message to IRC server */
	amount_array[0] = 5;
	amount_array[1] = (unsigned char) nick_length;
	amount_array[2] = 11;
	amount_array[3] = (unsigned char) ip_address_length;
	amount_array[4] = 2;
	amount_array[5] = amount_array[1];
	amount_array[6] = 2;
	bytes_copied = fast_strcat(buffer,amount_array,7,"USER ",nick," localhost ",ip_address," :",nick,"\r\n");
	printf("Wrote following to socket: %s\n",buffer);
	function_response = write(main_socket,buffer,bytes_copied);
	if(function_response == -1)
		exit_system_error("error writing to socket");

	/* receive message from server and print */
	function_response = recv(main_socket,buffer,BUFFER_LIMIT - 1,0);
	buffer[BUFFER_LIMIT - 1] = 0; /*unecessary*/
	buffer[function_response - 1] = 0;
	printf("%s\n",buffer);

	FD_ZERO(&set);
	FD_SET(0,&set);
	FD_SET(main_socket,&set);
	set_backup = set;

	/* Main loop */
	for (;;) {
		if(select(main_socket + 1,&set,NULL,NULL,NULL) == -1)
			exit_system_error("select failed");

		if(FD_ISSET(0,&set)) {
			if(!fgets(input,INPUT_LIMIT,stdin))
				exit_system_error("error with fgets");
			input_length = strlen(input) - 1;
			/* get rid of possible EOF stdin */
			input[input_length] = 0;
			
			now_time = time(NULL);
			if(!difftime(now_time,prior_time))
				printf("Slow down buddy\n");
			prior_time = now_time;


			if(parse_input_and_send_to_server(input,buffer,input_length,main_socket,nick,channel,&channel_length))
				exit(EXIT_SUCCESS);
		}

		if(FD_ISSET(main_socket,&set)) {
			/* TODO maybe peek until \r\n and then read after that, and process 
			 * TODO create function to handle server response*/
			function_response = recv(main_socket,buffer,BUFFER_LIMIT - 1,0);
			if(function_response == -1) { 
				exit_system_error("error with recv");
			} else if (!function_response) {
				close(main_socket);
				printf("Server closed connection\nexiting...\n");
				exit(EXIT_SUCCESS);
			} else if (function_response > BUFFER_LIMIT-3) {
				fprintf(stderr,"\n\nServer sent a message too close to the buffer "
						"limit: %d bytes, truncating message\n"
						"Consider raising BUFFER_LIMIT in main.c, "
						"the only penalty would be system memory\n\n",
						BUFFER_LIMIT);
				buffer[function_response - 1] = '\n';
				buffer[function_response - 2] = '\r';
			}	
#ifdef DEBUG
			printf("\nserver response length: %d bytes\n",function_response);
#endif
			buffer[function_response] = 0;
			printf("%s",buffer);
		}
		set = set_backup;
	}


	return 0;
}
