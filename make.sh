gcc src/main.c -c -I inc
gcc -Wall -Werror src/parser.c -c -I inc
gcc -Wall -Werror src/base64.c -c -I inc
gcc -Wall -Werror src/logger.c -c -I inc
gcc main.o parser.o base64.o logger.o -o server.out

rm main.o parser.o base64.o logger.o
