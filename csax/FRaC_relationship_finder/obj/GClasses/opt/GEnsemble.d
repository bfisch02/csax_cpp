../../obj/GClasses/opt/GEnsemble.o: GEnsemble.cpp GEnsemble.h GLearner.h GMatrix.h GError.h \
 GRand.h GVec.h GDistribution.h GNeuralNet.h GDom.h GHeap.h GHolders.h \
 GThread.h GDecisionTree.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GEnsemble.cpp -o ../../obj/GClasses/opt/GEnsemble.o
