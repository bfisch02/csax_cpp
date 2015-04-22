../../obj/GClasses/opt/GFourier.o: GFourier.cpp GFourier.h GError.h GHolders.h GMath.h GImage.h \
 GBits.h GVec.h GMatrix.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GFourier.cpp -o ../../obj/GClasses/opt/GFourier.o
