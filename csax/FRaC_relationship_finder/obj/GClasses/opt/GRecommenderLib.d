../../obj/GClasses/opt/GRecommenderLib.o: GRecommenderLib.cpp GRecommenderLib.h GActivation.h \
 GError.h GMath.h GApp.h GCluster.h GTransform.h GLearner.h GMatrix.h \
 GRand.h GDistance.h GFile.h GHolders.h GNeuralNet.h GRecommender.h \
 GNeighborFinder.h GSparseMatrix.h GDom.h GHeap.h usage.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GRecommenderLib.cpp -o ../../obj/GClasses/opt/GRecommenderLib.o
