all: runLibertyServer.exe 

clean:
	rm *.obj *.exe

runLibertyServer.exe: runLibertyServer.obj LibertyServer.obj ../SocketServer/SocketServer.obj 
	link "C:\Program Files (x86)\Polhemus\PDI\PDI_100\Lib\Win32\PDI.lib" $^ 

LibertyServer.obj: LibertyServer.cpp ..\SocketServer\SocketServer.h
	cl -c /EHsc -I'C:\Program Files (x86)\Polhemus\PDI\PDI_100\Inc' -I'..\SocketServer\' -I'..\inc' $<

runLibertyServer.obj: runLibertyServer.cpp ..\SocketServer\SocketServer.h
	cl -c /EHsc -I'C:\Program Files (x86)\Polhemus\PDI\PDI_100\Inc' -I'..\SocketServer\' -I'..\inc' $<

%.obj: %.cpp
	cl -c $<
