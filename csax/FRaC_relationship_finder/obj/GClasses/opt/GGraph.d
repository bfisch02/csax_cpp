../../obj/GClasses/opt/GGraph.o: GGraph.cpp GGraph.h GOptimizer.h GError.h GMatrix.h GBitTable.h \
 GHashTable.h GRegion.h GHeap.h GNeighborFinder.h GRand.h GVec.h \
 GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GGraph.cpp -o ../../obj/GClasses/opt/GGraph.o
