CXX      = i686-w64-mingw32-g++
CXXFLAGS = -std=c++14 -m32 -O2 -Wall -Wextra -Wno-unused-parameter \
           -Wno-strict-aliasing -Wno-cast-function-type -Wno-comment
INCLUDES = -Iplugin/src -Iplugin/include
LDFLAGS  = -shared -static-libgcc -static-libstdc++ -lkernel32 -luser32
DEFINES  = -D_H3API_PATCHER_X86_

SRC_DIR  = plugin/src
BUILD_DIR = build
OUTPUT   = $(BUILD_DIR)/H3.ClaudeExpo.dll

SOURCES  = $(SRC_DIR)/dllmain.cpp $(SRC_DIR)/H3ClaudeExpo.cpp
OBJECTS  = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean plugin install

all: plugin

plugin: $(OUTPUT)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(OUTPUT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Built: $(OUTPUT)"

clean:
	rm -rf $(BUILD_DIR)

# Copy the DLL to the HOMM3 HD Mod packs directory
# Set HOMM3_DIR to your HOMM3 installation path
HOMM3_DIR ?= $(HOME)/.wine/drive_c/GOG\ Games/HoMM\ 3\ Complete
PACK_DIR  = $(HOMM3_DIR)/_HD3_Data/Packs/H3.ClaudeExpo

install: plugin
	mkdir -p "$(PACK_DIR)"
	cp $(OUTPUT) "$(PACK_DIR)/"
	@echo "Installed to $(PACK_DIR)"
