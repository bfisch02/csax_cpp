../../obj/GClasses/opt/GBayesianNetwork.o: GBayesianNetwork.cpp GBayesianNetwork.h GHeap.h \
 GError.h GRand.h GMath.h GHolders.h GMatrix.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GBayesianNetwork.cpp -o ../../obj/GClasses/opt/GBayesianNetwork.o
