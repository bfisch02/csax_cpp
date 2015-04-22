../../obj/GClasses/opt/GSystemLearner.o: GSystemLearner.cpp GSystemLearner.h GMatrix.h GError.h \
 GActivation.h GMath.h GPolicyLearner.h GNeuralNet.h GLearner.h GRand.h \
 GNeighborFinder.h GHillClimber.h GOptimizer.h GVec.h GManifold.h \
 GTransform.h GFile.h GHeap.h GImage.h GDom.h GTime.h GTree.h \
 GEvolutionary.h GApp.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GSystemLearner.cpp -o ../../obj/GClasses/opt/GSystemLearner.o
