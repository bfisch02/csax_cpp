../../obj/GClasses/opt/sha1.o: sha1.cpp sha1.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c sha1.cpp -o ../../obj/GClasses/opt/sha1.o
