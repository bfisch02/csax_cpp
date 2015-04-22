../../obj/GClasses/opt/GError.o: GError.cpp GError.h GString.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GError.cpp -o ../../obj/GClasses/opt/GError.o
