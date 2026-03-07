BUILD = build
SRC = src
INCLUDE = include

CC = gcc
LD = ld

CC_FLAGS = -Wall -Wextra -g -O0 -I./$(INCLUDE)/ -D_GNU_SOURCE
LD_FLAGS = -g -fPIC -shared

TARGET_LIBIPC = $(BUILD)/libipc.so
TARGET_SERVER = $(BUILD)/server
TARGET_CLIENT1 = $(BUILD)/client1
TARGET_CLIENT2 = $(BUILD)/client2

COMMON_DEPS = $(SRC)/libipc.h $(SRC)/trace.h

LIBIPC_SRC = $(SRC)/libipc.c
LIBIPC_DEP = $(COMMON_DEPS) $(INCLUDE)/ipc_defs.h
LIBIPC_OBJS = $(BUILD)/libipc.o

SERVER_SRC = $(SRC)/server.c $(SRC)/argv.c $(SRC)/trace.c
SERVER_DEP = $(COMMON_DEPS) $(SRC)/argv.h $(SRC)/dtype.h

CLIENT1_SRC = $(SRC)/client1.c $(SRC)/argv.c $(SRC)/trace.c
CLIENT1_DEP = $(COMMON_DEPS) $(SRC)/argv.h $(SRC)/dtype.h

CLIENT2_SRC = $(SRC)/client2.c $(SRC)/argv.c $(SRC)/trace.c
CLIENT2_DEP = $(COMMON_DEPS) $(SRC)/argv.h $(SRC)/dtype.h

all: $(BUILD) server client1 client2

$(TARGET_CLIENT1): $(TARGET_LIBIPC) $(CLIENT1_SRC) $(CLIENT1_DEP)
	$(CC) $(CC_FLAGS) $(CLIENT1_SRC) -L$(BUILD) -lipc -o $@

$(TARGET_CLIENT2): $(CLIENT2_SRC) $(CLIENT2_DEP)
	$(CC) $(CC_FLAGS) $(CLIENT2_SRC) -o $@

$(TARGET_SERVER): $(TARGET_LIBIPC) $(SERVER_SRC) $(SERVER_DEP)
	$(CC) $(CC_FLAGS) $(SERVER_SRC) -L$(BUILD) -lipc -o $@

$(TARGET_LIBIPC): $(LIBIPC_OBJS)
	$(LD) $(LD_FLAGS) $(LIBIPC_OBJS) -o $@

$(LIBIPC_OBJS): $(LIBIPC_SRC) $(LIBIPC_DEP)
	$(CC) $(CC_FLAGS) -c -fPIC $(LIBIPC_SRC) -o $@

$(BUILD):
	mkdir $@

server: $(TARGET_SERVER)
client1: $(TARGET_CLIENT1)
client2: $(TARGET_CLIENT2)

clean:
	rm $(BUILD)/*

