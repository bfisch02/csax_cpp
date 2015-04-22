../../obj/GClasses/opt/GPolicyLearner.o: GPolicyLearner.cpp GPolicyLearner.h GMatrix.h GError.h \
 GKNN.h GLearner.h GRand.h GDecisionTree.h GNeighborFinder.h GOptimizer.h \
 GVec.h GHeap.h GHillClimber.h GDom.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GPolicyLearner.cpp -o ../../obj/GClasses/opt/GPolicyLearner.o
