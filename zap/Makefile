# TNL Makefile
# (c) 2003 GarageGames

# 
# Configuration
#
CC=g++ -g -I../tnl #-O2

OBJECTS_ZAP=\
	game.o\
	gameConnection.o\
	gameLoader.o\
	gameNetInterface.o\
	gameObject.o\
	gridDB.o\
	masterConnection.o\
	../master/masterInterface.o\
	UI.o\
	UIGame.o\
	UIMenus.o\
	UINameEntry.o\
	UIQueryServers.o\
	projectile.o\
	ship.o\
	SweptEllipsoid.o\
	sparkManager.o\
	teleporter.o\
	barrier.o\
	main.o


CFLAGS=

.cpp.o : 
	$(CC) -c $(CFLAGS) $<

default: $(OBJECTS_ZAP)
	$(CC) -o zap $(OBJECTS_ZAP) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a -lstdc++ -lGL -lGLU -lglut -lm

../master/masterInterface.o:
	make -C ../master

clean:
	rm -f $(OBJECTS_ZAP) zap

