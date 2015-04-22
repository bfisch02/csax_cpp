../../obj/GClasses/dbg/GDecisionTree.o: GDecisionTree.cpp GDecisionTree.h GLearner.h GMatrix.h \
 GError.h GRand.h GVec.h GPolynomial.h GHillClimber.h GOptimizer.h \
 GDistribution.h GDom.h GHeap.h GTransform.h GEnsemble.h GHolders.h
	g++ -Wall -g -D_DEBUG -c GDecisionTree.cpp -o ../../obj/GClasses/dbg/GDecisionTree.o
