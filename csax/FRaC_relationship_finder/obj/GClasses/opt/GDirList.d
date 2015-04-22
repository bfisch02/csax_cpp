../../obj/GClasses/opt/GDirList.o: GDirList.cpp GDirList.h GError.h GFile.h GBlob.h GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDirList.cpp -o ../../obj/GClasses/opt/GDirList.o
