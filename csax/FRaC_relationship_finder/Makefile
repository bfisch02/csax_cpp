#DEFINITIONS

#DIRECTORIES
SRC = src
LIB = lib
OBJ = obj$(mode)
BIN = bin$(mode)

#Make sure the directories exist
$(OBJ):
	mkdir $(OBJ)

$(BIN):
	mkdir $(BIN)

#CONDITIONALS

#Default: No linker optimization flags
LOPTFLAGS = 
#Default: Light optimization (-Og) and full line information (-g3).  Debug checks on (-DDEBUG).
OPTFLAGS = -O0 -g3 -DDEBUG -fopenmp -D_GLIBCXX_PARALLEL
#Define UNKNOWNVALUES for code that will support them.
CDEFS = -DUNKNOWNVALUES

WAFFLESMODE = dbg
WAFFLESLIB = GClassesDbg

OPTFLAGS_OPTIMIZED = -DNDEBUG -ffast-math -Ofast -fno-enforce-eh-specs -s -g0 -fno-math-errno -fmerge-all-constants -fno-ident -fno-stack-protector -ffunction-sections -fdata-sections -fno-rtti -fnothrow-opt -flto  -fopenmp -D_GLIBCXX_PARALLEL

#optflags for LTO
#LOPTFLAGS_OPTIMIZED = -Wl,-z,norelro -Wl,--hash-style=gnu -Wl,--build-id=none -flto=jobserver -Ofast -Wl,--gc-sections -Wl,--as-needed -fno-exceptions
LOPTFLAGS_OPTIMIZED = -Wl,-z,norelro -Wl,--hash-style=gnu -Wl,--build-id=none -flto=jobserver -Ofast -Wl,--gc-sections -Wl,--as-needed -fno-exceptions

ifeq ($(mode),release)
	OPTFLAGS = $(OPTFLAGS_OPTIMIZED)
	WAFFLESMODE = opt
	WAFFLESLIB = GClasses
	LOPTFLAGS = $(OPTFLAGS) $(LOPTFLAGS_OPTIMIZED)
endif

ifeq ($(mode),local)
	OPTFLAGS = $(OPTFLAGS_OPTIMIZED) -march=native
	WAFFLESMODE = opt
	WAFFLESLIB = GClasses
	LOPTFLAGS = $(OPTFLAGS) $(LOPTFLAGS_OPTIMIZED)
endif

ifeq ($(unknowns),nounknowns)
	CDEFS = 
endif

#FLAGS
CXX ?= g++
CFLAGS = -pipe -std=c++11 -Wall -fPIC -I waffles_src $(OPTFLAGS) $(CDEFS)
SHVER = 2
LINKER = -L $(LIB) -l$(WAFFLESLIB) -lm -lpthread $(LOPTFLAGS)

HEADERS = ${SRC}/array.hpp ${SRC}/vectormath.hpp
#RESOURCES

OBJECTS = ${OBJ}/svm.o ${OBJ}/frac.o ${OBJ}/sample.o ${OBJ}/errormodel.o ${OBJ}/errormodelcollection.o ${OBJ}/mltypes.o ${OBJ}/predictor.o ${OBJ}/waffles.o ${OBJ}/svmlearner.o ${OBJ}/io.o ${OBJ}/evaluation.o ${OBJ}/sampletransformer.o # ${OBJ}/yadt.o

#END DEFINITIONS

#General

FRaC: ${BIN}/FRaC
frac: FRaC

${BIN}/FRaC: ${OBJ}/main.o ${OBJECTS} lib | $(BIN)
ifneq (,$(filter $(mode),local release)) #Enter this block for local and release (but not for nothing or other).
  #Make the binary as usual, but also strip it to remove some unneeded data sections.
	$(CXX) $(CFLAGS) ${OBJ}/main.o ${OBJECTS} -o ${BIN}/FRaC ${LINKER};   strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag ${BIN}/FRaC
else
	$(CXX) $(CFLAGS) ${OBJ}/main.o ${OBJECTS} -o ${BIN}/FRaC ${LINKER}
endif

test: ${BIN}/test

${BIN}/test: ${SRC}/unittest.cpp ${OBJECTS} lib | $(BIN)
	$(CXX) $(CFLAGS) ${SRC}/unittest.cpp ${OBJECTS} -o ${BIN}/test ${LINKER}

#Library
ifeq ($(WAFFLESMODE),dbg)
lib: libsvm libwafflesdbg
else
lib: libsvm libwaffles
endif

libsvm:

#libsvm: ${OBJ}/svm.o
#	$(CXX) $(CFLAGS) -shared -dynamiclib ${OBJ}/svm.o -o ${OBJ}/libsvm.so.$(SHVER)

libwafflesdbg: $(LIB)/libGClassesDbg.a

lib/libGClassesDbg.a:
	cd waffles_src/GClasses;make dbg -j

libwaffles: $(LIB)/libGClasses.a

lib/libGClasses.a:
ifeq ($(dest),local)
	cd waffles_src/GClasses;make opt -j
else
	cd waffles_src/GClasses;make opt dest=local -j
endif
#Object Creation:

${OBJ}/main.o: ${SRC}/main.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/main.cpp -o ${OBJ}/main.o
  
${OBJ}/svm.o: ${SRC}/svm.cpp ${SRC}/svm.h $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/svm.cpp -o ${OBJ}/svm.o

${OBJ}/frac.o: ${SRC}/frac.hpp ${SRC}/frac.cpp ${SRC}/sample.hpp ${SRC}/errormodel.hpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/frac.cpp -o ${OBJ}/frac.o

${OBJ}/sample.o: ${SRC}/sample.hpp ${SRC}/sample.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/sample.cpp -o ${OBJ}/sample.o
	
${OBJ}/errormodel.o: ${SRC}/errormodel.hpp ${SRC}/errormodel.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/errormodel.cpp -o ${OBJ}/errormodel.o

${OBJ}/errormodelcollection.o: ${SRC}/errormodelcollection.hpp ${SRC}/errormodelcollection.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/errormodelcollection.cpp -o ${OBJ}/errormodelcollection.o

${OBJ}/mltypes.o: ${SRC}/mltypes.hpp ${SRC}/mltypes.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/mltypes.cpp -o ${OBJ}/mltypes.o

${OBJ}/predictor.o: ${SRC}/predictor.hpp ${SRC}/predictor.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/predictor.cpp -o ${OBJ}/predictor.o

${OBJ}/waffles.o: ${SRC}/waffles.hpp ${SRC}/waffles.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/waffles.cpp -o ${OBJ}/waffles.o

${OBJ}/svmlearner.o: ${SRC}/svmlearner.hpp ${SRC}/svmlearner.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/svmlearner.cpp -o ${OBJ}/svmlearner.o

${OBJ}/io.o: ${SRC}/io.hpp ${SRC}/io.cpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/io.cpp -o ${OBJ}/io.o

${OBJ}/evaluation.o: ${SRC}/evaluation.cpp ${SRC}/evaluation.hpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/evaluation.cpp -o ${OBJ}/evaluation.o

${OBJ}/sampletransformer.o: ${SRC}/sampletransformer.cpp ${SRC}/sampletransformer.hpp $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/sampletransformer.cpp -o ${OBJ}/sampletransformer.o


#TODO not using this anymore.
${OBJ}/yadt.o: ${SRC}/dTcmd.cpp ${SRC}/YaDT.h $(HEADERS) | $(OBJ)
	$(CXX) $(CFLAGS) -c ${SRC}/dTcmd.cpp -o ${OBJ}/yadt.o -I ./src/ 

#Documentation:

doc: doc/fracdesign.pdf

doc/fracdesign.pdf: doc/fracdesign.tex doc/bibliography.bib
	cd doc;sh makedoc.sh

#Major Rules:

all: FRaC test evaluation doc

cleanobj:
	rm -rf ${OBJ}*/*

cleanbin:
	rm -rf ${BIN}*/*

cleanlib:
	cd waffles_src/GClasses/;make clean -j

cleandoc:
	rm -f doc/fracdesign.aux doc/fracdesign.blg doc/fracdesign.log doc/fracdesign.out doc/fracdesign.toc doc/fracdesign.pdf

clean: cleandoc cleanobj cleanbin cleanlib
	rm -rf libsvm.so.$(SHVER) ${SRC}/*.ghc

build: clean all


distclean: clean
	rm -f frac.libsvr

build: clean all

help:
	echo "Options: [mode=release] for optimized code, [dest=local] for code optimized to run locally, [unknowns=nounknowns] for compiling without support for unknown values.";echo "Primary targets: FRaC test doc"


