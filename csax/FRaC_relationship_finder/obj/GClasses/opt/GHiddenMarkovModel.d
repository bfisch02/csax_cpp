../../obj/GClasses/opt/GHiddenMarkovModel.o: GHiddenMarkovModel.cpp GHiddenMarkovModel.h \
 GError.h GHolders.h GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHiddenMarkovModel.cpp -o ../../obj/GClasses/opt/GHiddenMarkovModel.o
