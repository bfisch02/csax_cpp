../../obj/GClasses/dbg/GCluster.o: GCluster.cpp GCluster.h GTransform.h GLearner.h GMatrix.h \
 GError.h GRand.h GNeighborFinder.h GDistance.h GBitTable.h GHeap.h \
 GMath.h GVec.h GHillClimber.h GOptimizer.h GSparseMatrix.h GKNN.h \
 GHolders.h GTime.h GTree.h GGraph.h GDom.h GImage.h
	g++ -Wall -g -D_DEBUG -c GCluster.cpp -o ../../obj/GClasses/dbg/GCluster.o
