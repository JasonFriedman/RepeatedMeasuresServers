all: bin/runLibertyServer.exe bin/LibertyClientExample.exe

bin/runLibertyServer.exe: LibertyServer/* SocketServer/*
	make -C SocketServer
	make -C LibertyServer
	cp LibertyServer/runLibertyServer.exe bin/

bin/LibertyClientExample.exe: LibertyClient/* SocketClient/*
	make -C SocketClient
	make -C LibertyClient
	cp LibertyClient/LibertyClientExample.exe bin/

