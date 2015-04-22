../../obj/GClasses/opt/GBigInt.o: GBigInt.cpp GBigInt.h GError.h GHolders.h GKeyPair.h GDom.h \
 GHeap.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GBigInt.cpp -o ../../obj/GClasses/opt/GBigInt.o
