../../obj/GClasses/opt/GTransform.o: GTransform.cpp GTransform.h GLearner.h GMatrix.h GError.h \
 GRand.h GDom.h GHeap.h GVec.h GDistribution.h GManifold.h GCluster.h \
 GString.h GNeuralNet.h GRecommender.h GApp.h GNeighborFinder.h \
 GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GTransform.cpp -o ../../obj/GClasses/opt/GTransform.o
