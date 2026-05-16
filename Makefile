PLUGIN_NAME = hyprland-follow-window.so
HYPRLAND_SRC ?= /path/to/Hyprland-0.55.0

CXXFLAGS = -shared -fPIC -g -std=c++2b -Wno-c++11-narrowing -O2
INCLUDES = `pkg-config --cflags pixman-1 libdrm pangocairo libinput libudev wayland-server xkbcommon`
LIBS = `pkg-config --libs pangocairo`

SRC = src/main.cpp

all: $(PLUGIN_NAME)

$(PLUGIN_NAME): $(SRC) src/globals.hpp
	@if [ ! -d "$(HYPRLAND_SRC)" ]; then echo "HYPRLAND_SRC does not exist: $(HYPRLAND_SRC)"; exit 1; fi
	$(CXX) $(CXXFLAGS) -I$(HYPRLAND_SRC) -I$(HYPRLAND_SRC)/src -I$(HYPRLAND_SRC)/protocols $(INCLUDES) $(SRC) -o $@ $(LIBS)

clean:
	rm -f ./$(PLUGIN_NAME)

.PHONY: all clean
