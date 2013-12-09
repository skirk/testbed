PROJECT=program
CXX=g++
CC=gcc

OPTS=-Wall -g -c  -std=c++0x
OPTSC=-Wall -g -c -std=c99

# folder structure
SRCDIR = src
OBJDIR = obj

LDFLAGS= $(shell sdl2-config --libs) \
	-L/usr/lib64/nvidia/ -lOpenCL -lGL -lGLEW -lSDL2_ttf

INCLUDEPATH = -I./include/ $(shell sdl2-config --cflags) -I/opt/cuda/include/
SRCS := $(shell find $(SRCDIR) -name '*.cpp')
SRCSC := $(shell find $(SRCDIR) -name '*.c')
SRCDIRS := $(shell find $(SRCDIR) -type d | sed 's/$(SRCDIR)/./g' )
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCSC))
OBJS += $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# link object files
$(PROJECT): all $(OBJS)
	   $(CXX) $(OBJS) $(LDFLAGS) -o $@

# create object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(OPTSC) $(INCLUDEPATH) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(OPTS) $(INCLUDEPATH) $< -o $@

clean:
	rm $(PROJECT) $(OBJDIR) -Rf

all:
	@$(call make-repo)

#create obj directory structure
define make-repo
mkdir -p $(OBJDIR)
	for dir in $(SRCDIRS); \
		do \
		mkdir -p $(OBJDIR)/$$dir; \
		done
endef
