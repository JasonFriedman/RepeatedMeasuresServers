all: SocketServer.obj

clean:
	rm *.obj 
	
%.obj: %.cpp
	cl /EHsc /I ../inc -c $<
