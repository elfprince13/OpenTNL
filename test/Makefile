# TNL Makefile
# (c) 2003 GarageGames
#
# This makefile is for Linux atm.


# 
# Configuration
#
CC=g++ -g #-O2

OBJECTS_SERVER=\
	testGame.o\
	server.o

OBJECTS_CLIENT=\
	testGame.o\
	client.o

OBJECTS_WXGUI=\
	testGame.o\
	testWindow.o \
	testGameRender.o \

CFLAGS=

.cpp.o : 
	$(CC) -c $(CFLAGS) $<

default: $(OBJECTS_SERVER) $(OBJECTS_CLIENT)
	@echo Building linux dedicated server...
	$(CC) -o server $(OBJECTS_SERVER) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a -lstdc++ -lm
	$(CC) -o client $(OBJECTS_CLIENT) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a -lstdc++ -lm

wxgui: CFLAGS += `wx-config --cxxflags`
wxgui: $(OBJECTS_WXGUI)
	@echo Building WX gui...
	$(CC) -o wxgui $(OBJECTS_WXGUI) ../tnl/libtnl.a ../libtomcrypt/libtomcrypt.a `wx-config --libs --gl-libs ` -lGL
clean:
	rm -f $(OBJECTS_SERVER) $(OBJECTS_CLIENT) $(OBJECTS_WXGUI) server client wxgui
