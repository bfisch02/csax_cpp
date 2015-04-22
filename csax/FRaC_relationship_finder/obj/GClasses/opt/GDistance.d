../../obj/GClasses/opt/GDistance.o: GDistance.cpp GDistance.h GMatrix.h GError.h GDom.h GHeap.h \
 GVec.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDistance.cpp -o ../../obj/GClasses/opt/GDistance.o
