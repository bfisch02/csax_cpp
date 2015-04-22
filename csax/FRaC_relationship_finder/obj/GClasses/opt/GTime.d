../../obj/GClasses/opt/GTime.o: GTime.cpp GTime.h GTree.h GError.h GString.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GTime.cpp -o ../../obj/GClasses/opt/GTime.o
