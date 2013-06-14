UNAME := $(shell uname)

CFLAGS := -Wall -g

ifeq ($(UNAME), Linux)
LUA_CFLAGS := $(shell pkg-config --cflags lua5.2)
LUA_LDFLAGS := $(shell pkg-config --libs lua5.2)
LDFLAGS += -lglut -lGLU -lGL -lm
endif

ifeq ($(UNAME), Darwin)
LUA_CFLAGS := -I/usr/local/include
LUA_LDFLAGS := -L/usr/local/lib -llua
LDFLAGS += -framework GLUT -framework OpenGL
endif

CFLAGS += $(LUA_CFLAGS)
LDFLAGS += $(LUA_LDFLAGS)

all: objview

headers := $(wildcard *.h)
sources := $(wildcard *.c)
objects := $(sources:%.c=%.o)

$(objects): $(headers)

objview: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f objview *.o
