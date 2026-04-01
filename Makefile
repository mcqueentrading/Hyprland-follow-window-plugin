PLUGIN_NAME = hyprland-follow-window.so
HYPRLAND_SRC ?= /path/to/fresh/Hyprland

CXXFLAGS = -shared -fPIC -g -std=c++2b -Wno-c++11-narrowing -O2
INCLUDES = `pkg-config --cflags pixman-1 libdrm pangocairo libinput libudev wayland-server xkbcommon`
LIBS = `pkg-config --libs pangocairo`

SRC = src/main.cpp

all: $(PLUGIN_NAME)

$(PLUGIN_NAME): $(SRC)
	@if [ ! -d "$(HYPRLAND_SRC)" ]; then echo "HYPRLAND_SRC does not exist: $(HYPRLAND_SRC)"; exit 1; fi
	$(CXX) $(CXXFLAGS) -I$(HYPRLAND_SRC) -I$(HYPRLAND_SRC)/src -I$(HYPRLAND_SRC)/protocols $(INCLUDES) $^ -o $@ $(LIBS)

clean:
	rm -f ./$(PLUGIN_NAME)

.PHONY: all clean
