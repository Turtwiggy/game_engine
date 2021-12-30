#
# Make the "Example" main
#

CC = gcc
CXX = g++

# build dirs
ASSETS_DIR = assets
ASSETS_HTML_TEMPLATE = assets/emscripten/shell_minimal.html
BUILD_DIR = build
OBJ_DIR = build-objs
EXE = build/hello.exe

# src dirs
PROJECT_DIR = example
ENGINE_DIR = engine
IMGUI_DIR = thirdparty/imgui
SDL2_DIR = thirdparty/sdl2/x86_64-w64-mingw32
GLEW_DIR = thirdparty/glew

# config..
DEBUG = 1
EMSCRIPTEN = 0
ifeq ($(EMSCRIPTEN), 1)
	EMSCRIPTEN = $(EMSCRIPTEN)
endif
$(info building emscripten... $(EMSCRIPTEN))

# vars..
CXXFLAGS =
LDFLAGS = 
LIBS = 

ifeq ($(EMSCRIPTEN), 1)
	CC = emcc
	CXX = em++
	WEB_DIR = build-web
	EXE = $(WEB_DIR)/index.html

	EMS += -s USE_SDL=2
	EMS += -s DISABLE_EXCEPTION_CATCHING=1
	LDFLAGS += -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1

	CXXFLAGS += -Os $(EMS)
	LDFLAGS += --shell-file $(ASSETS_HTML_TEMPLATE) $(EMS)
endif

#
# BUILD OPTIONS
#

ifeq ($(CXX), g++)
CXXFLAGS += -O2
endif

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -Wextra -pedantic
endif
CXXFLAGS += -std=c++17 -Wall -Wformat 

# INCLUDES
CXXFLAGS += -I$(PROJECT_DIR)
CXXFLAGS += -I$(ENGINE_DIR)
CXXFLAGS += -I$(IMGUI_DIR)
CXXFLAGS += -I$(IMGUI_DIR)/backends
# header-only libs
CXXFLAGS += -Ithirdparty/magic_enum
CXXFLAGS += -Ithirdparty/stb_image
# Don't include sdl2 or glew for emscripten
ifeq ($(EMSCRIPTEN), 0)
	CXXFLAGS += -I$(SDL2_DIR)/include/SDL2
	CXXFLAGS += -I$(GLEW_DIR)/include
endif

#
# SOURCE FILES
#

SOURCES = $(PROJECT_DIR)/main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addprefix $(OBJ_DIR)/,$(addsuffix .o, $(basename $(notdir $(SOURCES)))))

#
# BUILD FLAGS PER PLATFORM
#

ifeq ($(EMSCRIPTEN), 0)
# only link if not emscripten...

ifeq ($(OS), Windows_NT)
		LIBS += -L$(SDL2_DIR)/lib
		LIBS += -L$(GLEW_DIR)/lib/Release/x64
    LIBS += -lopengl32 -lSDL2main -lSDL2 -lglew32
endif

endif

#
# BUILD RULES
#

# Notes:
# $< is input file name
# $@ is output file name
# $^ is all the inputs
# $(outputs) all the outputs

# %.o:%.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:$(PROJECT_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:$(ENGINE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:$(ENGINE_DIR)/opengl/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(EXE)
	$(info Build complete for: $(EXE))

ifeq ($(EMSCRIPTEN), 0)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

else

$(EXE): $(OBJS) $(WEB_DIR)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) 

endif

#
# CREATE DIRS
#

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir "$(BUILD_DIR)" "$(OBJ_DIR)" "$(WEB_DIR)"

#
# EMSCRIPTEN
#

$(WEB_DIR):
	rmdir -r $@
	mkdir $@

serve: all
	python3 -m http.server -d $(WEB_DIR)

clean:
	rm -r $(OBJS) $(WEB_DIR)

#
# HELPER
#

copy_assets: $(BUILD_DIR)
	$(info copying assets...)
	copy -r assets $(BUILD_DIR)/assets

copy_dlls: $(BUILD_DIR)
	$(info copying dlls...)
	copy thirdparty\sdl2\x86_64-w64-mingw32\bin\SDL2.dll $(BUILD_DIR)
	copy thirdparty\glew\bin\Release\x64\glew32.dll $(BUILD_DIR)