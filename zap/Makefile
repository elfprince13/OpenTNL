# TNL Makefile
# (c) 2003 GarageGames

# 
# Configuration
#
CC=g++ -g -I../tnl -I../glut -I../openal -DTNL_DEBUG -DTNL_ENABLE_LOGGING #-O2

OBJECTS_ZAP=\
	CTFGame.o\
	game.o\
	gameConnection.o\
	gameItems.o\
	gameLoader.o\
	gameNetInterface.o\
	gameObject.o\
	gameType.o\
	gridDB.o\
	item.o\
	masterConnection.o\
	../master/masterInterface.o\
	moveObject.o\
	UI.o\
	UIGame.o\
	UIMenus.o\
	UINameEntry.o\
	UIQueryServers.o\
	projectile.o\
	quickChat.o\
	ship.o\
	sfx.o\
	SweptEllipsoid.o\
	sparkManager.o\
	teleporter.o\
	barrier.o\
	main.o


CFLAGS=

.cpp.o : 
	$(CC) -c $(CFLAGS) $<

default: $(OBJECTS_ZAP)
	$(CC) -o zap $(OBJECTS_ZAP) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a ../openal/linux/libopenal.a -lpthread -lstdc++ -lGL -lGLU -lglut -lm

../master/masterInterface.o:
	make -C ../master

clean:
	rm -f $(OBJECTS_ZAP) zap

