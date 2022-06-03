# CLI-IRC-client

A simple and very portable IRC client.  Should compile with common compilers on Linux and other Unix-like systems.  Tested on GCC and compile script uses GCC. The software is C89 compliant and doesn't use any functions outside ANSI C and POSIX sockets API.

## To compile and run
git clone https://github.com/orchardstreet/CLI-IRC-client

cd CLI-IRC-client

chmod +x compile.sh

./compile.sh

./main IP PORT NICK

eg ./main 127.0.0.1 6667 chris
