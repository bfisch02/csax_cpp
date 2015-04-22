../../obj/GClasses/opt/GText.o: GText.cpp GText.h GHashTable.h GError.h GHeap.h GStemmer.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GText.cpp -o ../../obj/GClasses/opt/GText.o
