../../obj/GClasses/opt/GHolders.o: GHolders.cpp GHolders.h GError.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHolders.cpp -o ../../obj/GClasses/opt/GHolders.o
