../../obj/GClasses/opt/GKeyPair.o: GKeyPair.cpp GKeyPair.h GBigInt.h GError.h GRand.h GDom.h \
 GHeap.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GKeyPair.cpp -o ../../obj/GClasses/opt/GKeyPair.o
