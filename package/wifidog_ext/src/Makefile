#Makefile 


BIN_DIR:=./bin
BIN_BINDNAME:=$(BIN_DIR)/bindname
BIN_BINDSUC:=$(BIN_DIR)/bindsuc
BIN_RJYQ:=$(BIN_DIR)/rjyq
BIN_MYTHSTART:=$(BIN_DIR)/mythstart
BIN_ALLOW:=$(BIN_DIR)/allow
BIN_UPDAEMON:=$(BIN_DIR)/upgrade_daemon
#all bin
BIN_ALL:=$(BIN_BINDNAME) $(BIN_BINDSUC) $(BIN_RJYQ) $(BIN_MYTHSTART) $(BIN_ALLOW) $(BIN_UPDAEMON)

SRC_BINDNAME:=bindname.c
SRC_BINDSUC:=bindstatus.c
SRC_RJYQ:=b_client.c tcpclient.c
SRC_MYTHSTART:=gateway_monitor_openwrt.c
SRC_ALLOW:=makeallowip.c
SRC_UPDAEMON:=upgrade_daemon.c

ifeq ($(CC),)
CC:=gcc
endif

all: dir $(BIN_ALL)

dir:
	mkdir -p $(BIN_DIR)

$(BIN_BINDNAME):$(SRC_BINDNAME)
	$(CC) -o $@ $?
	
$(BIN_BINDSUC):$(SRC_BINDSUC)
	$(CC) -o $@ $?
	
$(BIN_RJYQ):$(SRC_RJYQ)
	$(CC) $(CPPFLAGS) -o $@ $? 
	
$(BIN_MYTHSTART):$(SRC_MYTHSTART)
	$(CC) -o $@ $?
	
$(BIN_ALLOW):$(SRC_ALLOW)
	$(CC) -o $@ $?

$(BIN_UPDAEMON):$(SRC_UPDAEMON)
	$(CC) -o $@ $?
	
.PHONY:
clean:
	rm -rf *.o $(BIN_DIR)
