../../obj/GClasses/opt/GDom.o: GDom.cpp GDom.h GError.h GHeap.h GFile.h GHolders.h GTokenizer.h \
 GBitTable.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GDom.cpp -o ../../obj/GClasses/opt/GDom.o
