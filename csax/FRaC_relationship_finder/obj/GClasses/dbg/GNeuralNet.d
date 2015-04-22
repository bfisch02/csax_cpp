../../obj/GClasses/dbg/GNeuralNet.o: GNeuralNet.cpp GNeuralNet.h GLearner.h GMatrix.h GError.h \
 GRand.h GMath.h GActivation.h GDistribution.h GVec.h GDom.h GHeap.h \
 GHillClimber.h GOptimizer.h GTransform.h GSparseMatrix.h GDistance.h \
 GAssignment.h GHolders.h GBits.h GFourier.h
	g++ -Wall -g -D_DEBUG -c GNeuralNet.cpp -o ../../obj/GClasses/dbg/GNeuralNet.o
