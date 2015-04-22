../../obj/GClasses/opt/GLinear.o: GLinear.cpp GLinear.h GLearner.h GMatrix.h GError.h GRand.h \
 GDom.h GHeap.h GTransform.h GDistribution.h GVec.h GOptimizer.h \
 GHillClimber.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GLinear.cpp -o ../../obj/GClasses/opt/GLinear.o
