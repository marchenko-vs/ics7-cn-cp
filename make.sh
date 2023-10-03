gcc -Wall -Werror src/main.c -c -I inc
gcc -Wall -Werror src/parser.c -c -I inc
gcc -Wall -Werror src/base64.c -c -I inc
gcc main.o parser.o base64.o -o server.out

rm main.o parser.o base64.o
