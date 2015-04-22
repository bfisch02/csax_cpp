../../obj/GClasses/dbg/GKNN.o: GKNN.cpp GKNN.h GLearner.h GMatrix.h GError.h GRand.h GDom.h \
 GHeap.h GDistribution.h GVec.h GNeighborFinder.h GHillClimber.h \
 GOptimizer.h GCluster.h GTransform.h GBitTable.h GDistance.h \
 GSparseMatrix.h GHolders.h
	g++ -Wall -g -D_DEBUG -c GKNN.cpp -o ../../obj/GClasses/dbg/GKNN.o
