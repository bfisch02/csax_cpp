../../obj/GClasses/opt/GRayTrace.o: GRayTrace.cpp GRayTrace.h G3D.h GError.h GRand.h \
 GHashTable.h GDom.h GHeap.h GImage.h GMath.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GRayTrace.cpp -o ../../obj/GClasses/opt/GRayTrace.o
