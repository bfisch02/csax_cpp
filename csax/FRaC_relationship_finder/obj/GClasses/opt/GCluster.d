../../obj/GClasses/opt/GCluster.o: GCluster.cpp GCluster.h GTransform.h GLearner.h GMatrix.h \
 GError.h GRand.h GNeighborFinder.h GDistance.h GBitTable.h GHeap.h \
 GMath.h GVec.h GHillClimber.h GOptimizer.h GSparseMatrix.h GKNN.h \
 GHolders.h GTime.h GTree.h GGraph.h GDom.h GImage.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GCluster.cpp -o ../../obj/GClasses/opt/GCluster.o
