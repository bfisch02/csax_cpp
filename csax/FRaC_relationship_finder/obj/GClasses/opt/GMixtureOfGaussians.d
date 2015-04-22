../../obj/GClasses/opt/GMixtureOfGaussians.o: GMixtureOfGaussians.cpp GMixtureOfGaussians.h \
 GDistribution.h GVec.h GError.h GMatrix.h GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GMixtureOfGaussians.cpp -o ../../obj/GClasses/opt/GMixtureOfGaussians.o
