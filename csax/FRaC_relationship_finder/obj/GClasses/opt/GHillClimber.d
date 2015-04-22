../../obj/GClasses/opt/GHillClimber.o: GHillClimber.cpp GHillClimber.h GOptimizer.h GError.h \
 GMatrix.h GVec.h GRand.h GImage.h GBitTable.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHillClimber.cpp -o ../../obj/GClasses/opt/GHillClimber.o
