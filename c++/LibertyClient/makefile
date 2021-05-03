all: LibertyClientExample.exe

clean:
	rm *.obj *.exe

LibertyClientExample.exe: LibertyClientExample.obj LibertyClient.obj ../SocketClient/SocketClient.obj
	link $^

%.obj: %.cpp
	cl /I ../inc /I ../SocketClient/ -c $<
