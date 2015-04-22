../../obj/GClasses/opt/GFunction.o: GFunction.cpp GFunction.h GError.h GHolders.h GMath.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GFunction.cpp -o ../../obj/GClasses/opt/GFunction.o
