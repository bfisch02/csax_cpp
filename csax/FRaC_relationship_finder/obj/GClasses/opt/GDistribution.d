../../obj/GClasses/opt/GDistribution.o: GDistribution.cpp GDistribution.h GVec.h GError.h GDom.h \
 GHeap.h GRand.h GMath.h GMatrix.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDistribution.cpp -o ../../obj/GClasses/opt/GDistribution.o
