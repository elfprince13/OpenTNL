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
	UICredits.o\
	projectile.o\
	quickChat.o\
	ship.o\
	sfx.o\
	soccerGame.o\
	SweptEllipsoid.o\
	sparkManager.o\
	teleporter.o\
	barrier.o\
	voiceCodec.o\
	lpc10dec.o\
	lpc10enc.o\
	gsm_decode.o\
	gsm_encode.o\
	gsm_state.o\
	main.o


CFLAGS=

.c.o:
	$(CC) -c $(CFLAGS) $<

.cpp.o : 
	$(CC) -c $(CFLAGS) $<

default: ../exe/zap

zap: ../exe/zap

zapded: ../exe/zapded

../exe/zap: $(OBJECTS_ZAP)
	$(CC) -o ../exe/zap $(OBJECTS_ZAP) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a ../openal/linux/libopenal.a -lpthread -lstdc++ -lGL -lGLU -lglut -lm

../exe/zapded $(OBJECTS_ZAP) CFLAGS=$(CFLAGS) -DZAP_DEDICATED
	$(CC) -o ../exe/zap $(OBJECTS_ZAP) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a ../openal/linux/libopenal.a -lpthread -lstdc++ -lm

../master/masterInterface.o:
	make -C ../master

clean:
	rm -f $(OBJECTS_ZAP) ../exe/zap

