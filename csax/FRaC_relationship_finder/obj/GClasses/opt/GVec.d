../../obj/GClasses/opt/GVec.o: GVec.cpp GVec.h GRand.h GError.h GMatrix.h GBits.h GDom.h GHeap.h \
 GMath.h GImage.h GBitTable.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GVec.cpp -o ../../obj/GClasses/opt/GVec.o
