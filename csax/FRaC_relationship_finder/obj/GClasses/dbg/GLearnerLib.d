../../obj/GClasses/dbg/GLearnerLib.o: GLearnerLib.cpp GLearnerLib.h GActivation.h GError.h \
 GMath.h GApp.h GMatrix.h GCluster.h GTransform.h GLearner.h GRand.h \
 GDecisionTree.h GDistance.h GDistribution.h GVec.h GEnsemble.h GFile.h \
 GFunction.h GGaussianProcess.h GHillClimber.h GOptimizer.h GHolders.h \
 GImage.h GKernelTrick.h GDom.h GHeap.h GKNN.h GLinear.h GManifold.h \
 GNaiveBayes.h GNaiveInstance.h GNeuralNet.h GSparseMatrix.h \
 GSystemLearner.h GTime.h GTree.h usage.h
	g++ -Wall -g -D_DEBUG -c GLearnerLib.cpp -o ../../obj/GClasses/dbg/GLearnerLib.o
