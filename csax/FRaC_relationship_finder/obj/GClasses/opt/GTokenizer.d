../../obj/GClasses/opt/GTokenizer.o: GTokenizer.cpp GTokenizer.h GBitTable.h GError.h GHolders.h \
 GFile.h GString.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GTokenizer.cpp -o ../../obj/GClasses/opt/GTokenizer.o
