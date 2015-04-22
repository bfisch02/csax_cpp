../../obj/GClasses/opt/G3D.o: G3D.cpp G3D.h GError.h GRand.h GDom.h GHeap.h GMath.h GImage.h \
 GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c G3D.cpp -o ../../obj/GClasses/opt/G3D.o
