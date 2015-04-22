../../obj/GClasses/opt/GMatrix.o: GMatrix.cpp GMatrix.h GError.h GAssignment.h GHolders.h \
 GMath.h GDistribution.h GVec.h GFile.h GHashTable.h GBits.h \
 GNeighborFinder.h GDistance.h GHeap.h GDom.h GLearner.h GRand.h \
 GTokenizer.h GBitTable.h GTime.h GTree.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GMatrix.cpp -o ../../obj/GClasses/opt/GMatrix.o
