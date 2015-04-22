../../obj/GClasses/opt/GDecisionTree.o: GDecisionTree.cpp GDecisionTree.h GLearner.h GMatrix.h \
 GError.h GRand.h GVec.h GPolynomial.h GHillClimber.h GOptimizer.h \
 GDistribution.h GDom.h GHeap.h GTransform.h GEnsemble.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDecisionTree.cpp -o ../../obj/GClasses/opt/GDecisionTree.o
