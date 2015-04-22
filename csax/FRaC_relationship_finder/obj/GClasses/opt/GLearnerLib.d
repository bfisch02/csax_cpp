../../obj/GClasses/opt/GLearnerLib.o: GLearnerLib.cpp GLearnerLib.h GActivation.h GError.h \
 GMath.h GApp.h GMatrix.h GCluster.h GTransform.h GLearner.h GRand.h \
 GDecisionTree.h GDistance.h GDistribution.h GVec.h GEnsemble.h GFile.h \
 GFunction.h GGaussianProcess.h GHillClimber.h GOptimizer.h GHolders.h \
 GImage.h GKernelTrick.h GDom.h GHeap.h GKNN.h GLinear.h GManifold.h \
 GNaiveBayes.h GNaiveInstance.h GNeuralNet.h GSparseMatrix.h \
 GSystemLearner.h GTime.h GTree.h usage.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GLearnerLib.cpp -o ../../obj/GClasses/opt/GLearnerLib.o
