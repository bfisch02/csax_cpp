../../obj/GClasses/opt/GNaiveInstance.o: GNaiveInstance.cpp GNaiveInstance.h GLearner.h \
 GMatrix.h GError.h GRand.h GVec.h GDom.h GHeap.h GDistribution.h \
 GTransform.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GNaiveInstance.cpp -o ../../obj/GClasses/opt/GNaiveInstance.o
