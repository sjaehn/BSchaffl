SHELL = /bin/sh

BUNDLE = BSlizr.lv2
VER = 0.4

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

PKG_CONFIG ?= pkg-config
CXX ?= g++
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC -DPIC
LDFLAGS += -shared

DSPFLAGS =
GUIFLAGS = -DPUGL_HAVE_CAIRO

DSPFLAGS += `$(PKG_CONFIG) --cflags --libs lv2`
GUIFLAGS += `$(PKG_CONFIG) --cflags --libs lv2 x11 cairo`

GUI_INCL = \
	src/BWidgets/DrawingSurface.cpp \
	src/BWidgets/DisplayDial.cpp \
	src/BWidgets/Dial.cpp \
	src/BWidgets/DisplayVSlider.cpp \
	src/BWidgets/VSlider.cpp \
	src/BWidgets/VScale.cpp \
	src/BWidgets/HSwitch.cpp \
	src/BWidgets/DisplayHSlider.cpp \
	src/BWidgets/HSlider.cpp \
	src/BWidgets/HScale.cpp \
	src/BWidgets/RangeWidget.cpp \
	src/BWidgets/ValueWidget.cpp \
	src/BWidgets/Knob.cpp \
	src/BWidgets/Label.cpp \
	src/BWidgets/FocusWidget.cpp \
	src/BWidgets/Window.cpp \
	src/BWidgets/Widget.cpp \
	src/BWidgets/BStyles.cpp \
	src/BWidgets/BColors.cpp \
	src/BWidgets/BEvents.cpp \
	src/BWidgets/BValues.cpp \
	src/screen.c \
	src/BWidgets/cairoplus.c \
	src/BWidgets/pugl/pugl_x11_cairo.c \
	src/BWidgets/pugl/pugl_x11.c

$(BUNDLE): clean BSlizr.so BSlizr_GUI.so
	cp manifest.ttl BSlizr.ttl surface.png LICENSE $(BUNDLE)

all: $(BUNDLE)

BSlizr.so: ./src/BSlizr.cpp
	mkdir -p $(BUNDLE)
	$(CXX) $< -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)

BSlizr_GUI.so: ./src/BSlizr_GUI.cpp
	mkdir -p $(BUNDLE)
	$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)

install:
	mkdir -p $(DESTDIR)$(LV2DIR)
	rm -rf $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)

.PHONY: all

clean:
	rm -rf $(BUNDLE)
