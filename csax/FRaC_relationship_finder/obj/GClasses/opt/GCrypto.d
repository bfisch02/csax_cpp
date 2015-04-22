../../obj/GClasses/opt/GCrypto.o: GCrypto.cpp GCrypto.h GError.h GHolders.h GRand.h sha1.h \
 sha2.h uitypes.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GCrypto.cpp -o ../../obj/GClasses/opt/GCrypto.o
