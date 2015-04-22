../../obj/GClasses/opt/GBits.o: GBits.cpp GBits.h GError.h GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GBits.cpp -o ../../obj/GClasses/opt/GBits.o
