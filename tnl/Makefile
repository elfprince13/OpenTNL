# TNL Makefile
# (c) 2003 GarageGames
#
# This makefile is for gcc-based projects, at the moment.


# 
# Configuration
#
CC=gcc -g -DTNL_DEBUG -I../libtomcrypt # -O2

OBJECTS=\
	assert.o\
	asymmetricKey.o\
	bitStream.o\
	byteBuffer.o\
	clientPuzzle.o\
	connectionStringTable.o\
	dataChunker.o\
	eventConnection.o\
	ghostConnection.o\
	huffmanStringProcessor.o\
	log.o\
	netBase.o\
	netConnection.o\
	netInterface.o\
	netObject.o\
	netStringTable.o\
	platform.o\
	random.o\
	rpc.o\
	symmetricCipher.o\
	udp.o\
	vector.o\

CFLAGS=

.cpp.o : 
	$(CC) -c $(CFLAGS) $<

default: $(OBJECTS)
	@echo Building libtnl.a ...
	ar rcv libtnl.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libtnl.a
