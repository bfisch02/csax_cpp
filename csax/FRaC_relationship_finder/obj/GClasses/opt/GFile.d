../../obj/GClasses/opt/GFile.o: GFile.cpp GFile.h GError.h GHolders.h GString.h GApp.h \
 GBitTable.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GFile.cpp -o ../../obj/GClasses/opt/GFile.o
