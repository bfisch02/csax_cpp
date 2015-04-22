../../obj/GClasses/opt/GImage.o: GImage.cpp GImage.h GError.h GBitTable.h GBits.h GFile.h \
 GRect.h GFourier.h GOptimizer.h GMatrix.h GHillClimber.h GVec.h GRand.h \
 GMath.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GImage.cpp -o ../../obj/GClasses/opt/GImage.o
