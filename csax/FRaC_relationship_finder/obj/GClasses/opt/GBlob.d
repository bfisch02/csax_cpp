../../obj/GClasses/opt/GBlob.o: GBlob.cpp GBlob.h GError.h GBits.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GBlob.cpp -o ../../obj/GClasses/opt/GBlob.o
