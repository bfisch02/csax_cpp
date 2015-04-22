../../obj/GClasses/opt/GNeuralNet.o: GNeuralNet.cpp GNeuralNet.h GLearner.h GMatrix.h GError.h \
 GRand.h GMath.h GActivation.h GDistribution.h GVec.h GDom.h GHeap.h \
 GHillClimber.h GOptimizer.h GTransform.h GSparseMatrix.h GDistance.h \
 GAssignment.h GHolders.h GBits.h GFourier.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GNeuralNet.cpp -o ../../obj/GClasses/opt/GNeuralNet.o
