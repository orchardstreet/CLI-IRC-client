#ifndef NETWORK
#define NETWORK

unsigned short get_port_from_port_string(struct String *port_string);

signed char connect_to_server(int *current_socket,struct String *ip_address, unsigned short port);

#endif
