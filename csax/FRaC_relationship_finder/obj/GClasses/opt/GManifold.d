../../obj/GClasses/opt/GManifold.o: GManifold.cpp GManifold.h GTransform.h GLearner.h GMatrix.h \
 GError.h GRand.h GVec.h GActivation.h GMath.h GBits.h GBitTable.h \
 GGraph.h GOptimizer.h GHillClimber.h GHeap.h GImage.h GKNN.h GLinear.h \
 GNeighborFinder.h GNeuralNet.h GPlot.h GRect.h GSparseMatrix.h GTime.h \
 GTree.h GDom.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GManifold.cpp -o ../../obj/GClasses/opt/GManifold.o
