../../obj/GClasses/opt/GSelfOrganizingMap.o: GSelfOrganizingMap.cpp GSelfOrganizingMap.h \
 GError.h GTransform.h GLearner.h GMatrix.h GRand.h GHolders.h GVec.h \
 GMath.h GImage.h GDistance.h GDom.h GHeap.h \
 GSelfOrganizingMapTestData.cpp
	g++ -DNDEBUG -ffast-math -Ofast -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections  -fnothrow-opt  -march=native -c GSelfOrganizingMap.cpp -o ../../obj/GClasses/opt/GSelfOrganizingMap.o
