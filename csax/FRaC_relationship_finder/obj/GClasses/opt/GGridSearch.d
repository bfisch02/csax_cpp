../../obj/GClasses/opt/GGridSearch.o: GGridSearch.cpp GGridSearch.h GOptimizer.h GError.h \
 GMatrix.h GRand.h GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GGridSearch.cpp -o ../../obj/GClasses/opt/GGridSearch.o
