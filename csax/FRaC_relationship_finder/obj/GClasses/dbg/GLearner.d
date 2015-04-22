../../obj/GClasses/dbg/GLearner.o: GLearner.cpp GLearner.h GMatrix.h GError.h GRand.h GVec.h \
 GHeap.h GDom.h GGaussianProcess.h GImage.h GNeuralNet.h GKNN.h \
 GDecisionTree.h GNaiveInstance.h GLinear.h GNaiveBayes.h GEnsemble.h \
 GPolynomial.h GTransform.h GHolders.h GPlot.h GRect.h GMath.h \
 GDistribution.h GRecommender.h GApp.h GNeighborFinder.h
	g++ -Wall -g -D_DEBUG -c GLearner.cpp -o ../../obj/GClasses/dbg/GLearner.o
