../../obj/GClasses/opt/GRegion.o: GRegion.cpp GRegion.h GImage.h GError.h GHeap.h GFourier.h \
 GBits.h GRect.h GVec.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GRegion.cpp -o ../../obj/GClasses/opt/GRegion.o
