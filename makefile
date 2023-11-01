CC := gcc
CFLAGS := -Wall -Werror -std=gnu99
SRC_DIR := src
INC_DIR := inc
OBJ_DIR := out

server.out : $(OBJ_DIR) $(OBJ_DIR)/main.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/server.o
	$(CC) $(OBJ_DIR)/*.o -o server.out

$(OBJ_DIR) :
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/main.o : $(SRC_DIR)/main.c $(INC_DIR)/logger.h $(INC_DIR)/server.h
	$(CC) $(CFLAGS) -I $(INC_DIR) $(SRC_DIR)/main.c -c -o $(OBJ_DIR)/main.o

$(OBJ_DIR)/logger.o : $(SRC_DIR)/logger.c $(INC_DIR)/logger.h
	$(CC) $(CFLAGS) -I $(INC_DIR) $(SRC_DIR)/logger.c -c -o $(OBJ_DIR)/logger.o

$(OBJ_DIR)/server.o : $(SRC_DIR)/server.c $(INC_DIR)/server.h $(INC_DIR)/logger.h
	$(CC) $(CFLAGS) -I $(INC_DIR) $(SRC_DIR)/server.c -c -o $(OBJ_DIR)/server.o

.PHONY : clean

clean :
	rm -rf $(OBJ_DIR) server.out