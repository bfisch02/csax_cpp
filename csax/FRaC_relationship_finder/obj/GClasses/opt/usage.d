../../obj/GClasses/opt/usage.o: usage.cpp usage.h GError.h GString.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c usage.cpp -o ../../obj/GClasses/opt/usage.o
