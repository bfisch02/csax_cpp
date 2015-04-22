../../obj/GClasses/opt/GBezier.o: GBezier.cpp GBezier.h GError.h GMath.h G3D.h GHolders.h \
 GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GBezier.cpp -o ../../obj/GClasses/opt/GBezier.o
