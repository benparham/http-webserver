CC = gcc

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

TARGET = serv

INC = $(shell find $(INC_DIR) -type f -name '*.h')
SRC = $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ = $(subst .c,.o,$(subst $(SRC_DIR),$(BUILD_DIR),$(SRC)))

CFLAGS = -g -Wall -std=c99
INCFLAGS = -I $(INC_DIR)


# Rules #######################

all: $(TARGET)

# Makes the server executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Makes the server object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCFLAGS) -c -o $@ $<

clean:
	@rm -f $(TARGET)
	@rm -rf $(BUILD_DIR)