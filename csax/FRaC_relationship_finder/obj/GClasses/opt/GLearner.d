../../obj/GClasses/opt/GLearner.o: GLearner.cpp GLearner.h GMatrix.h GError.h GRand.h GVec.h \
 GHeap.h GDom.h GGaussianProcess.h GImage.h GNeuralNet.h GKNN.h \
 GDecisionTree.h GNaiveInstance.h GLinear.h GNaiveBayes.h GEnsemble.h \
 GPolynomial.h GTransform.h GHolders.h GPlot.h GRect.h GMath.h \
 GDistribution.h GRecommender.h GApp.h GNeighborFinder.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GLearner.cpp -o ../../obj/GClasses/opt/GLearner.o
