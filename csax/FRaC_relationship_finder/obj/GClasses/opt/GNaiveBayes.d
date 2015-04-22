../../obj/GClasses/opt/GNaiveBayes.o: GNaiveBayes.cpp GNaiveBayes.h GLearner.h GMatrix.h \
 GError.h GRand.h GVec.h GDom.h GHeap.h GDistribution.h GTransform.h \
 GSparseMatrix.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GNaiveBayes.cpp -o ../../obj/GClasses/opt/GNaiveBayes.o
