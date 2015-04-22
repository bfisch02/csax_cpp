../../obj/GClasses/opt/GSocket.o: GSocket.cpp GSocket.h GError.h GDom.h GHeap.h GHolders.h \
 GThread.h GRand.h GApp.h GTime.h GTree.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GSocket.cpp -o ../../obj/GClasses/opt/GSocket.o
