../../obj/GClasses/dbg/GPolicyLearner.o: GPolicyLearner.cpp GPolicyLearner.h GMatrix.h GError.h \
 GKNN.h GLearner.h GRand.h GDecisionTree.h GNeighborFinder.h GOptimizer.h \
 GVec.h GHeap.h GHillClimber.h GDom.h
	g++ -Wall -g -D_DEBUG -c GPolicyLearner.cpp -o ../../obj/GClasses/dbg/GPolicyLearner.o
