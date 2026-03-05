BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include

CC = gcc
LD = ld

CC_FLAGS = -Wall -Wextra -g -O3 -I./$(INCLUDE_DIR)/
LD_FLAGS = -g -fPIC -shared

TARGET_LIBIPC = $(BUILD_DIR)/libipc.so
TARGET_SERVER = $(BUILD_DIR)/server
TARGET_CLIENT1 = $(BUILD_DIR)/client1
TARGET_CLIENT2 = $(BUILD_DIR)/client2

COMMON_DEPS = $(SRC_DIR)/libipc.h $(INCLUDE_DIR)/ipc_defs.h

LIBIPC_SRC = $(SRC_DIR)/libipc.c
LIBIPC_DEP = $(COMMON_DEPS)
LIBIPC_OBJS = $(BUILD_DIR)/libipc.o

SERVER_SRC = $(SRC_DIR)/server.c $(SRC_DIR)/argv.c $(SRC_DIR)/trace.c
SERVER_OBJS = $(BUILD_DIR)/server.o
SERVER_DEP = $(COMMON_DEPS) $(SRC_DIR)/argv.h $(SRC_DIR)/dtype.h

CLIENT1_SRC = $(SRC_DIR)/client1.c $(SRC_DIR)/argv.c $(SRC_DIR)/trace.c
CLIENT1_OBJS = $(BUILD_DIR)/client1.o
CLIENT1_DEP = $(COMMON_DEPS) $(SRC_DIR)/argv.h $(SRC_DIR)/dtype.h

CLIENT2_SRC = $(SRC_DIR)/client2.c $(SRC_DIR)/argv.c $(SRC_DIR)/trace.c
CLIENT2_OBJS = $(BUILD_DIR)/client2.o
CLIENT2_DEP = $(COMMON_DEPS) $(SRC_DIR)/argv.h $(SRC_DIR)/dtype.h

all: $(BUILD_DIR) server client1 client2

$(TARGET_CLIENT1): $(TARGET_LIBIPC) $(CLIENT1_SRC) $(CLIENT1_DEP)
	$(CC) $(CC_FLAGS) $(CLIENT1_SRC) -L$(BUILD_DIR) -lipc -o $@

$(TARGET_CLIENT2): $(CLIENT2_SRC) $(CLIENT2_DEP)
	$(CC) $(CC_FLAGS) $(CLIENT2_SRC) -o $@

$(TARGET_SERVER): $(TARGET_LIBIPC) $(SERVER_SRC) $(SERVER_DEP)
	$(CC) $(CC_FLAGS) $(SERVER_SRC) -L$(BUILD_DIR) -lipc -o $@

$(TARGET_LIBIPC): $(LIBIPC_OBJS)
	$(LD) $(LD_FLAGS) $(LIBIPC_OBJS) -o $@

$(LIBIPC_OBJS): $(LIBIPC_SRC) $(LIBIPC_DEP)
	$(CC) $(CC_FLAGS) -c -fPIC $(LIBIPC_SRC) -o $@

$(BUILD_DIR):
	mkdir $@

server: $(TARGET_SERVER)
client1: $(TARGET_CLIENT1)
client2: $(TARGET_CLIENT2)

clean:
	rm $(BUILD_DIR)/*

