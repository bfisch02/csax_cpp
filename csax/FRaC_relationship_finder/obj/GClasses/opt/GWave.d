../../obj/GClasses/opt/GWave.o: GWave.cpp GWave.h GError.h GMath.h GFourier.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GWave.cpp -o ../../obj/GClasses/opt/GWave.o
