../../obj/GClasses/dbg/GRecommenderLib.o: GRecommenderLib.cpp GRecommenderLib.h GActivation.h \
 GError.h GMath.h GApp.h GCluster.h GTransform.h GLearner.h GMatrix.h \
 GRand.h GDistance.h GFile.h GHolders.h GNeuralNet.h GRecommender.h \
 GNeighborFinder.h GSparseMatrix.h GDom.h GHeap.h usage.h
	g++ -Wall -g -D_DEBUG -c GRecommenderLib.cpp -o ../../obj/GClasses/dbg/GRecommenderLib.o
