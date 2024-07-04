# -------------------------------------------------------
#  Projeto desenvolvido por:
#
#  Grupo 20:
#  Rodrigo Correia   58180
#  Laura Cunha       58188 
#  André Reis        58192
# -------------------------------------------------------

# DIRECTORIES
BIN_DIR = binary
INC_DIR = include
LIB_DIR = lib
OBJ_DIR = object
SRC_DIR = source
DEP_DIR = dependencies
LIBS = -ltable -lprotobuf-c -lzookeeper_mt

CC = gcc
CFLAGS = -g -D THREADED -Wall -MMD -MP -MF $(DEP_DIR)/$*.d -I $(INC_DIR) -L $(LIB_DIR)

PROTO_CC = protoc

# PATHS
vpath libtable.a $(LIB_DIR)
vpath table-client $(BIN_DIR)
vpath table-server $(BIN_DIR)

TARGETS = libtable.a table-client table-server

LIBTABLE_OBJS = $(OBJ_DIR)/data.o $(OBJ_DIR)/entry.o $(OBJ_DIR)/list.o $(OBJ_DIR)/table.o
CLIENT_OBJS = $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/table_client.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/message.o $(OBJ_DIR)/zoo_client.o $(OBJ_DIR)/zoo_common.o
SERVER_OBJS = $(OBJ_DIR)/sdmessage.pb-c.o $(OBJ_DIR)/synchronization.o $(OBJ_DIR)/table_server.o $(OBJ_DIR)/network_server.o $(OBJ_DIR)/table_skel.o $(OBJ_DIR)/message.o $(OBJ_DIR)/client_stub.o $(OBJ_DIR)/network_client.o $(OBJ_DIR)/zoo_server.o $(OBJ_DIR)/zoo_common.o

# TARGETS
all: $(TARGETS)

libtable.a: $(LIBTABLE_OBJS)
	ar -rcs $(LIB_DIR)/$@ $^

table-client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@ $(LIBS)

table-server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $(BIN_DIR)/$@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/sdmessage.pb-c.c: sdmessage.proto
	$(PROTO_CC) --c_out=$(SRC_DIR) $<
	mv $(SRC_DIR)/sdmessage.pb-c.h $(INC_DIR)

include $(wildcard $(DEP_DIR)/*.d)

.PHONY: folders
folders:
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
	mkdir -p $(OBJ_DIR)
	mkdir -p $(DEP_DIR)

.PHONY: clean
clean:
	rm -rf $(DEP_DIR)/* $(OBJ_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*