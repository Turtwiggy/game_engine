#
# Make the "Example" main
#

EXE = hello.exe
CC = gcc
CXX = g++

PROJECT_DIR = example
ENGINE_DIR = engine
IMGUI_DIR = thirdparty/imgui
SDL2_DIR = thirdparty/sdl2/x86_64-w64-mingw32
GLEW_DIR = thirdparty/glew

CXXFLAGS =
LIBS = 

#
# BUILD OPTIONS
#

CXXFLAGS += -std=c++17 -Wall -Wformat -Os 
CXXFLAGS += -I$(PROJECT_DIR)
CXXFLAGS += -I$(ENGINE_DIR)
CXXFLAGS += -I$(IMGUI_DIR)
CXXFLAGS += -I$(IMGUI_DIR)/backends
CXXFLAGS += -I$(SDL2_DIR)/include/SDL2
CXXFLAGS += -I$(GLEW_DIR)/include
# header-only libs
CXXFLAGS += -Ithirdparty/magic_enum
CXXFLAGS += -Ithirdparty/stb_image

#
# SOURCE FILES
#

SOURCES = $(PROJECT_DIR)/main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

#
# BUILD FLAGS PER PLATFORM
#

ifeq ($(OS), Windows_NT)
		LIBS += -L$(SDL2_DIR)/lib
		LIBS += -L$(GLEW_DIR)/lib/Release/x64
    LIBS += -lopengl32 -lSDL2main -lSDL2 -lglew32
endif

#
# BUILD RULES
#

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o:$(PROJECT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(ENGINE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(ENGINE_DIR)/opengl/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	$(info Build complete for: $(EXE))

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -r $(OBJS)