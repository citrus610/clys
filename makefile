CXX = g++

ifeq ($(PROF), true)
CXXPROF += -pg -no-pie
else
CXXPROF += -s
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -fdiagnostics-color=always -DUNICODE -std=c++20 -Wall -Og -pg -no-pie
else
CXXFLAGS += -DUNICODE -DNDEBUG -std=c++20 -O3 -msse4 -mbmi2 -flto $(CXXPROF) -march=native
endif

ifeq ($(PEXT), true)
CXXFLAGS += -DPEXT
endif

STATIC_LIB = -Bstatic -lsetupapi -lhid -luser32 -lgdi32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -static -static-libgcc

SRC_AI = core/*.cpp ai/*.cpp

SDL_DIR = D:/c++/lib/sdl/x86_64-w64-mingw32
SDL_INC = -I$(SDL_DIR)/include/SDL2
SDL_LIB = -L$(SDL_DIR)/lib -lmingw32 -lSDL2main -lSDL2 -Wl,--dynamicbase -Wl,--nxcompat -Wl,--high-entropy-va -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid

GUI_DIR = D:/c++/lib/imgui
GUI_INC = -I$(GUI_DIR) -I$(GUI_DIR)/backends
GUI_SRC = $(GUI_DIR)/*.cpp $(GUI_DIR)/backends/imgui_impl_sdl2.cpp $(GUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

PPT_NAME = ppt
PPT_SRC = $(SRC_AI)
PPT_LIB = $(STATIC_LIB) lib/ppt_sync/libppt_sync.dll.lib

ifeq ($(GUI), true)
CXXFLAGS += -DGUI
PPT_NAME = ppt_gui
PPT_SRC += $(SDL_INC) $(GUI_INC) $(GUI_SRC)
PPT_LIB += $(SDL_LIB)
endif

.PHONY: all cli tbp clean makedir

all: cli

cli: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) cli/*.cpp -o bin/cli/cli.exe

tbp: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) tbp/*.cpp -o bin/tbp/tbp.exe

ppt: makedir
	@$(CXX) $(CXXFLAGS) $(PPT_SRC) ppt/*.cpp $(PPT_LIB) -o bin/ppt/$(PPT_NAME).exe

tuner: makedir
	@$(CXX) $(CXXFLAGS) $(SRC_AI) tuner/*.cpp -o bin/tuner/tuner.exe

clean: makedir
	@rm -rf bin
	@make makedir

makedir:
	@mkdir -p bin
	@mkdir -p bin/cli
	@mkdir -p bin/tbp
	@mkdir -p bin/ppt
	@mkdir -p bin/tuner/data
	@cp lib/ppt_sync/libppt_sync.dll bin/ppt/libppt_sync.dll
	@cp lib/ppt_sync/ppt-sync.exe bin/ppt/ppt-sync.exe

.DEFAULT_GOAL := cli