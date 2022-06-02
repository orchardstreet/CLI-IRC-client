/* CLI IRC Client */
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define BUFFER_LIMIT 2000
#define INPUT_LIMIT 515
#include <stdarg.h>

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

unsigned char parse_input_and_send_to_server(char *input_browse,char *buf_browse,size_t input_length,int main_socket,char *channel,unsigned char *channel_length,char *nick) {
	/* return 0 on success, and 1 on quit */
	
	unsigned char amount_array[10];
	unsigned char bytes_copied = 0;
	char *buffer = buf_browse;
	char *input = input_browse;
	ssize_t function_response;
	size_t message_length;

	if(input_length > 512) {
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
				if(*input_browse != '#') {
					fprintf(stderr,"Error: Wrong syntax\nSyntax: /j #room\n");
					return 0; 
				}
				if(*(input_browse + 1) < 33 || *(input_browse + 1) > 126) {
					fprintf(stderr,"Error: invalid room name\n");
					return 0;
				}
				*channel_length = input_length - (input_browse - input);
				memcpy(channel,input_browse,*channel_length + 1);
				printf("channel length: %d",(int)*channel_length);
				memcpy(buf_browse,"JOIN ",5);
				buf_browse += 5;
				break;
			case 'q':
				input_browse++;
				if(*input_browse != 0 && *input_browse != ' ' && (strstr(input_browse,"uit") != input_browse)) { 
					fprintf(stderr,"Error: Not a valid command\n");
					return 0;
				}
				return 1;
			default:
				fprintf(stderr,"%c is not a command",*input_browse);
				return 0;
		}

	}
	input_length = input_length - (input_browse - input);
	message_length = (buf_browse - buffer) + input_length + 2;  
	memcpy(buf_browse,input_browse,input_length);
	buf_browse = buf_browse + input_length;
	memcpy(buf_browse,"\r\n",2);
	function_response = write(main_socket,buffer,message_length);
	printf("wrote: %ld",function_response);
	if(function_response == -1)
		exit_system_error("write failed");
	buffer[message_length] = 0;
	printf("s: %ld\n",message_length);
	printf("b: %s\n",buffer);
	/*function_response = write(fileno(stdout),buffer,message_length);*/
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
		fprintf(stderr,"couldn't establish connection");
		return -1;
	}


	printf("Connected to %s, port %d\n",ip_address,port);
	return 0;

}

int main(int argc, char *argv[])
{

	/* define variables */
	char buffer[BUFFER_LIMIT];
	char input[INPUT_LIMIT];
	int main_socket;
	int function_response;
	char channel[256];
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

	if(argc < 4)
		exit_error("Wrong syntax\nSyntax: ./main "
				"IP_ADDRESS PORT NICK");

	ip_address = argv[1];
	port_string = argv[2];
	nick = argv[3];
	channel[0] = 0;

	if((nick_length = strlen(nick)) > 9)
		exit_error("nick must be less than 10 characters");

	if((ip_address_length = strlen(ip_address)) > 39)
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

	fd_set set, set_backup;
	FD_ZERO(&set);
	FD_SET(fileno(stdin),&set);
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

		if(parse_input_and_send_to_server(input,buffer,input_length,main_socket,channel,&channel_length,nick))
			exit(EXIT_SUCCESS);
		}

		if(FD_ISSET(main_socket,&set)) {
			function_response = recv(main_socket,buffer,BUFFER_LIMIT - 1,0);
			if(function_response == -1) { 
				exit_system_error("error with recv");
			} else if (!function_response) {
				close(main_socket);
			}
			buffer[BUFFER_LIMIT - 1] = 0;
			printf("o: %d\n",function_response);
			buffer[function_response - 1] = 0;
			printf("yay\n");
			printf("%s\n",buffer);
		}
		set = set_backup;
	}



	return 0;
}
