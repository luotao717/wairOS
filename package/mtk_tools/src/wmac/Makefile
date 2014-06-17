#Makefile 


BIN_DIR:=../bin
BIN_WMAC:=$(BIN_DIR)/writemac

#all bin
BIN_ALL:=$(BIN_WMAC)

SRC_WMAC:=flash_api.c writemac.c

ifeq ($(CC),)
CC:=gcc
endif

all: dir $(BIN_ALL)

dir:
	mkdir -p $(BIN_DIR)

$(BIN_WMAC):$(SRC_WMAC)
	$(CC) $(CPPFLAGS) -o $@ $? 

.PHONY:
clean:
	rm -rf *.o $(BIN_DIR)
