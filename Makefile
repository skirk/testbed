PROJ=testbed

CC=g++

CFLAGS= -Wall -g -DDEBUG -lrt

# Check for 32-bit vs 64-bit
PROC_TYPE = $(strip $(shell uname -m | grep 64))
 
# Check for Mac OS
OS = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
DARWIN = $(strip $(findstring DARWIN, $(OS)))

# MacOS System
LIBS=-L/usr/lib64/nvidia/ -lOpenCL
CFLAGS+=-m64


INC_DIRS=/opt/cuda/include/

$(PROJ): $(PROJ).cpp
	$(CC) $(CFLAGS) -o $@ $^ $(INC_DIRS:%=-I%) $(LIB_DIRS:%=-L%) $(LIBS)

.PHONY: clean

clean:
	rm $(PROJ)
