../../obj/GClasses/opt/GActivation.o: GActivation.cpp GActivation.h GError.h GMath.h GDom.h \
 GHeap.h GRand.h GTime.h GTree.h GNeuralNet.h GLearner.h GMatrix.h GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GActivation.cpp -o ../../obj/GClasses/opt/GActivation.o
