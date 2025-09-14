
CC = gcc

CFLAGS = -Wall			 	\
	-DG_DISABLE_DEPRECATED 	 	\
	-DGDK_DISABLE_DEPRECATED 	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED

fifteen: main.c 
	gcc fifteen-puzzle.c -o fifteen-puzzle -Wall `pkg-config gtk+-3.0 --cflags --libs`

clean: 
	rm -f *.o fifteen
