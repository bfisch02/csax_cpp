../../obj/GClasses/opt/GPriorityQueue.o: GPriorityQueue.cpp GPriorityQueue.h GError.h GHolders.h \
 GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GPriorityQueue.cpp -o ../../obj/GClasses/opt/GPriorityQueue.o
