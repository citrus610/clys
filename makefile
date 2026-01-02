CXX ?= g++

ifeq ($(OS), Windows_NT)
	SUFFIX := .exe
	STATIC := -lstdc++fs -static -static-libgcc
else
	SUFFIX :=
	STATIC :=
endif

ifeq ($(PROF), true)
	CXXPROF += -pg -no-pie
else
	CXXPROF += -s
endif

ifeq ($(DEBUG), true)
	CXXFLAGS += -fdiagnostics-color=always -DUNICODE -std=c++20 -Wall -pthread -Og -g -no-pie
else
	CXXFLAGS += -fdiagnostics-color=always -DUNICODE -DNDEBUG -std=c++20 -Wall -O3 -pthread -flto $(CXXPROF)
endif

ifeq ($(PEXT), true)
	CXXFLAGS += -DPEXT
endif

SRC_AI = core/*.cpp ai/*.cpp
STATIC_LIB = -Bstatic -lsetupapi -lpsapi -lhid -luser32 -lgdi32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -static-libgcc

.PHONY: all tbp ppt clean makedir

all: tbp

tbp: makedir
	@$(CXX) $(CXXFLAGS) -march=native $(SRC_AI) tbp/*.cpp $(STATIC) -o bin/tbp/tbp$(SUFFIX)

ppt: makedir
	@$(CXX) $(CXXFLAGS) -march=native $(SRC_AI) ppt/*.cpp -o bin/ppt/ppt.exe $(STATIC_LIB)

tuner: makedir
	@$(CXX) $(CXXFLAGS) -march=native $(SRC_AI) tuner/*.cpp -o bin/tuner/tuner.exe

clean:
	@rm -rf bin

makedir:
	@mkdir -p bin
	@mkdir -p bin/tbp
	@mkdir -p bin/ppt
	@mkdir -p bin/tuner

.DEFAULT_GOAL := tbp