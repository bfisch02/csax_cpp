../../obj/GClasses/opt/GKNN.o: GKNN.cpp GKNN.h GLearner.h GMatrix.h GError.h GRand.h GDom.h \
 GHeap.h GDistribution.h GVec.h GNeighborFinder.h GHillClimber.h \
 GOptimizer.h GCluster.h GTransform.h GBitTable.h GDistance.h \
 GSparseMatrix.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GKNN.cpp -o ../../obj/GClasses/opt/GKNN.o
