../../obj/GClasses/opt/GParticleSwarm.o: GParticleSwarm.cpp GParticleSwarm.h GOptimizer.h \
 GError.h GMatrix.h GVec.h GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GParticleSwarm.cpp -o ../../obj/GClasses/opt/GParticleSwarm.o
