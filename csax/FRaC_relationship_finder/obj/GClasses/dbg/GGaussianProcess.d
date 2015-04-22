../../obj/GClasses/dbg/GGaussianProcess.o: GGaussianProcess.cpp GGaussianProcess.h GMatrix.h \
 GError.h GLearner.h GRand.h GDom.h GHeap.h GVec.h GDistribution.h \
 GKernelTrick.h GHolders.h
	g++ -Wall -g -D_DEBUG -c GGaussianProcess.cpp -o ../../obj/GClasses/dbg/GGaussianProcess.o
