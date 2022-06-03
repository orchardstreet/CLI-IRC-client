# CLI-IRC-client

A simple and very portable IRC client.  Should compile with gcc and other common compilers on Linux and Unix-like systems. The software is C89 compliant and doesn't use any functions outside ANSI C and POSIX sockets API.

## To compile and run
git clone https://github.com/orchardstreet/CLI-IRC-client

cd CLI-IRC-client

chmod +x compile.sh

./compile.sh

./main IP PORT NICK

eg ./main 127.0.0.1 6667 chris
