../../obj/GClasses/opt/sha2.o: sha2.cpp sha2.h uitypes.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c sha2.cpp -o ../../obj/GClasses/opt/sha2.o
