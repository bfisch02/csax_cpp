../../obj/GClasses/opt/GReinforcement.o: GReinforcement.cpp GReinforcement.h GPolicyLearner.h \
 GMatrix.h GError.h GLearner.h GRand.h GVec.h GNeuralNet.h GKNN.h \
 GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GReinforcement.cpp -o ../../obj/GClasses/opt/GReinforcement.o
