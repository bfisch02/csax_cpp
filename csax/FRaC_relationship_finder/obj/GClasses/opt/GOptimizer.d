../../obj/GClasses/opt/GOptimizer.o: GOptimizer.cpp GOptimizer.h GError.h GMatrix.h GVec.h \
 GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GOptimizer.cpp -o ../../obj/GClasses/opt/GOptimizer.o
