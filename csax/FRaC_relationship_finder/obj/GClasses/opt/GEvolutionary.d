../../obj/GClasses/opt/GEvolutionary.o: GEvolutionary.cpp GError.h GEvolutionary.h GOptimizer.h \
 GMatrix.h GBits.h GRand.h GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GEvolutionary.cpp -o ../../obj/GClasses/opt/GEvolutionary.o
