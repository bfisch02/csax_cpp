../../obj/GClasses/opt/GKernelTrick.o: GKernelTrick.cpp GKernelTrick.h GLearner.h GMatrix.h \
 GError.h GRand.h GVec.h GDom.h GHeap.h GHillClimber.h GOptimizer.h \
 GDistribution.h GMath.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GKernelTrick.cpp -o ../../obj/GClasses/opt/GKernelTrick.o
