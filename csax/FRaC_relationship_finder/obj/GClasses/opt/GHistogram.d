../../obj/GClasses/opt/GHistogram.o: GHistogram.cpp GHistogram.h GMatrix.h GError.h GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHistogram.cpp -o ../../obj/GClasses/opt/GHistogram.o
