../../obj/GClasses/opt/GString.o: GString.cpp GString.h GError.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GString.cpp -o ../../obj/GClasses/opt/GString.o
