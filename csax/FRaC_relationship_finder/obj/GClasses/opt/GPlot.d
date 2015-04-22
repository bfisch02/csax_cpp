../../obj/GClasses/opt/GPlot.o: GPlot.cpp GPlot.h GRect.h GError.h GMath.h GVec.h GImage.h \
 GRand.h
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GPlot.cpp -o ../../obj/GClasses/opt/GPlot.o
