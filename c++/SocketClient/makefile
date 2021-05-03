all: SocketClient.obj

clean:
	rm *.obj 
	
%.obj: %.cpp
	cl /I ../inc -c $<
