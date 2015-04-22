../../obj/GClasses/opt/GWidgets.o: GWidgets.cpp GHolders.h GWidgets.h GRect.h GError.h GImage.h \
 GDirList.h GFile.h GHeap.h GTime.h GTree.h GBits.h GWave.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GWidgets.cpp -o ../../obj/GClasses/opt/GWidgets.o
