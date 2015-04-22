../../obj/GClasses/opt/GSparseMatrix.o: GSparseMatrix.cpp GError.h GSparseMatrix.h GMatrix.h \
 GVec.h GFile.h GRand.h GHolders.h GDom.h GHeap.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GSparseMatrix.cpp -o ../../obj/GClasses/opt/GSparseMatrix.o
