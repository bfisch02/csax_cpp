../../obj/GClasses/opt/GHashTable.o: GHashTable.cpp GHashTable.h GError.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GHashTable.cpp -o ../../obj/GClasses/opt/GHashTable.o
