../../obj/GClasses/dbg/GSystemLearner.o: GSystemLearner.cpp GSystemLearner.h GMatrix.h GError.h \
 GActivation.h GMath.h GPolicyLearner.h GNeuralNet.h GLearner.h GRand.h \
 GNeighborFinder.h GHillClimber.h GOptimizer.h GVec.h GManifold.h \
 GTransform.h GFile.h GHeap.h GImage.h GDom.h GTime.h GTree.h \
 GEvolutionary.h GApp.h GHolders.h
	g++ -Wall -g -D_DEBUG -c GSystemLearner.cpp -o ../../obj/GClasses/dbg/GSystemLearner.o
