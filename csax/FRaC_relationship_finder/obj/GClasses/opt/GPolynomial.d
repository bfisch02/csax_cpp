../../obj/GClasses/opt/GPolynomial.o: GPolynomial.cpp GError.h GPolynomial.h GLearner.h \
 GMatrix.h GRand.h GVec.h GDistribution.h GMath.h GHillClimber.h \
 GOptimizer.h GDom.h GHeap.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GPolynomial.cpp -o ../../obj/GClasses/opt/GPolynomial.o
