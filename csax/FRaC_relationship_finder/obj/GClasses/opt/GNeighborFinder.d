../../obj/GClasses/opt/GNeighborFinder.o: GNeighborFinder.cpp GNeighborFinder.h GMatrix.h \
 GError.h GVec.h GRand.h GPlot.h GRect.h GMath.h GOptimizer.h \
 GHillClimber.h GGraph.h GBitTable.h GDom.h GHeap.h GKNN.h GLearner.h \
 GHolders.h GTransform.h GDistance.h GPriorityQueue.h GImage.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GNeighborFinder.cpp -o ../../obj/GClasses/opt/GNeighborFinder.o
