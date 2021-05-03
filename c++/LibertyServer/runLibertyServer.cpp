#define _WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "LibertyServer.h"
#include "SocketServer.h"

#include <string>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Must specify the port to listen on as the first argument\n");
		exit(-1);
	}
		
	printf("Initilizing the liberty server . . .\n");
	LibertyServer(0);
	printf("Done\n");

	char* port = argv[1];
	
	listen(port);
}