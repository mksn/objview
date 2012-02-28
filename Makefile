objects = main.o \
		  model_obj.o\
			model_iqm.o\
			shader.o \
		  image.o \
			vector.o

sources = main.c \
		  model_obj.c \
			model_idm.c\
		  image.c \
			shader.c \
			vector.c


all: objview

objview: $(objects)
	gcc -m32 -ggdb -framework OpenGL -framework GLUT -L/usr/local/lib -L/usr/X11/lib -o objview $(objects)

%.o : %.c $(wildcard *.h)
	gcc -m32 -ggdb -Wall -I. -I/usr/X11/include -I/usr/local/include -c $<

clean:
	rm -f objview
	rm -f *.o*
