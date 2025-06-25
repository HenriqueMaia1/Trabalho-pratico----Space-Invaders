ALLEGRO_VERSION=5.0.10
MINGW_VERSION=4.7.0
FOLDER=C:

FOLDER_NAME=/allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=/lib/liballegro-$(ALLEGRO_VERSION)-monolith-mt.a
INCLUDE_ALLEGRO=/include

CFLAGS=-I$(PATH_ALLEGRO)$(INCLUDE_ALLEGRO)
LFLAGS=$(PATH_ALLEGRO)$(LIB_ALLEGRO)

all: invaders.exe invadersCOMPLETO.exe



invadersCOMPLETO.exe: invadersCOMPLETO.o
	gcc -o invadersCOMPLETO.exe invadersCOMPLETO.o $(LFLAGS)

invadersCOMPLETO.o: invadersCOMPLETO.c
	gcc $(CFLAGS) -c invadersCOMPLETO.c		
	
invaders.exe: invaders.o
	gcc -o invaders.exe invaders.o $(LFLAGS)

invaders.o: invaders.c
	gcc $(CFLAGS) -c invaders.c		
	
	
clean:
	del *.o 
	del *.exe
