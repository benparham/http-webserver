CC = gcc

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

TARGET = serv

INC = $(shell find $(INC_DIR) -type f -name '*.h')
# LIB = $(shell find $(MODULES_DIR) -type f -name '*.a')
SRC = $(SRC_DIR)/serv.c
OBJ = $(BUILD_DIR)/serv.o

CFLAGS = -g -Wall -std=c99
# INCFLAGS = -I $(MODULES_DIR)/options -I $(MODULES_DIR)/transfer
# LIBFLAGS = -L $(MODULES_DIR)/options -loptions -L $(MODULES_DIR)/transfer -ltransfer


# Rules #######################

all: $(TARGET)

# Makes the server executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Makes the server object files
$(OBJ): $(SRC) $(INC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm $(TARGET)
	@rm -rf $(BUILD_DIR)