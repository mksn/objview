objects =	main.o \
			model-iqm.o\
			shader.o \
			image.o \
			vector.o
		  #model_obj.o\

sources =	main.c \
			model-iqm.c\
			image.c \
			shader.c \
			vector.c
		  #model_obj.c \


all: objview

objview: $(objects)
	gcc -m32 -ggdb -fno-omit-frame-pointer -framework OpenGL -framework GLUT -L/usr/local/lib -L/usr/X11/lib -o objview $(objects)


%.o : %.c $(wildcard *.h)
	gcc -m32 -ggdb -fno-omit-frame-pointer -Wall -I. -I/usr/X11/include -I/usr/local/include -c $<

clean:
	rm -f objview
	rm -f *.o*
