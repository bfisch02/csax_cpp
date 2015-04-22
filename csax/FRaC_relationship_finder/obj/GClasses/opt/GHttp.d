../../obj/GClasses/opt/GHttp.o: GHttp.cpp GFile.h GHttp.h GSocket.h GError.h GDom.h GHeap.h \
 GString.h GHolders.h GTime.h GTree.h GThread.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHttp.cpp -o ../../obj/GClasses/opt/GHttp.o
