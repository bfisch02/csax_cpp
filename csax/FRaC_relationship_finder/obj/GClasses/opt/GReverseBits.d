../../obj/GClasses/opt/GReverseBits.o: GReverseBits.cpp GError.h GReverseBits.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GReverseBits.cpp -o ../../obj/GClasses/opt/GReverseBits.o
