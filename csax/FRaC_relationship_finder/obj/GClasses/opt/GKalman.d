../../obj/GClasses/opt/GKalman.o: GKalman.cpp GKalman.h GMatrix.h GError.h GVec.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GKalman.cpp -o ../../obj/GClasses/opt/GKalman.o
