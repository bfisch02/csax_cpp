../../obj/GClasses/opt/GDynamicPage.o: GDynamicPage.cpp GDynamicPage.h GHttp.h GSocket.h \
 GError.h GDom.h GHeap.h GApp.h GBits.h GHashTable.h GImage.h GHolders.h \
 GFile.h GThread.h GString.h GTime.h GTree.h GRand.h sha1.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDynamicPage.cpp -o ../../obj/GClasses/opt/GDynamicPage.o
