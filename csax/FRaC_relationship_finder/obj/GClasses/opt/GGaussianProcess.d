../../obj/GClasses/opt/GGaussianProcess.o: GGaussianProcess.cpp GGaussianProcess.h GMatrix.h \
 GError.h GLearner.h GRand.h GDom.h GHeap.h GVec.h GDistribution.h \
 GKernelTrick.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GGaussianProcess.cpp -o ../../obj/GClasses/opt/GGaussianProcess.o
