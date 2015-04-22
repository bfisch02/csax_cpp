../../obj/GClasses/opt/GAssignment.o: GAssignment.cpp GMatrix.h GError.h GAssignment.h \
 GHolders.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GAssignment.cpp -o ../../obj/GClasses/opt/GAssignment.o
