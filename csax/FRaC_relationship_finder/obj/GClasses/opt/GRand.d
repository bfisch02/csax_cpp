../../obj/GClasses/opt/GRand.o: GRand.cpp GRand.h GError.h GHistogram.h GMatrix.h GTime.h \
 GTree.h GMath.h GVec.h GReverseBits.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GRand.cpp -o ../../obj/GClasses/opt/GRand.o
