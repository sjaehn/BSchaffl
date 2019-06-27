SHELL = /bin/sh

BUNDLE = BSlizr.lv2
VER = 0.4

PREFIX ?= /usr/local
LV2DIR ?= $(PREFIX)/lib/lv2

PKG_CONFIG ?= pkg-config
CXX ?= g++
CPPFLAGS += -DPIC
CXXFLAGS += -std=c++11 -fvisibility=hidden -fPIC
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

ifeq ($(shell $(PKG_CONFIG) --exists lv2 || echo no), no)
  $(error LV2 not found. Please install LV2 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists x11 || echo no), no)
  $(error X11 not found. Please install X11 first.)
endif

ifeq ($(shell $(PKG_CONFIG) --exists cairo || echo no), no)
  $(error Cairo not found. Please install cairo first.)
endif

$(BUNDLE): clean BSlizr.so BSlizr_GUI.so
	@cp manifest.ttl BSlizr.ttl surface.png LICENSE $(BUNDLE)

all: $(BUNDLE)

BSlizr.so: ./src/BSlizr.cpp
	@echo -n Build $(BUNDLE) DSP...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(DSPFLAGS)
	@echo \ done.

BSlizr_GUI.so: ./src/BSlizr_GUI.cpp
	@echo -n Build $(BUNDLE) GUI...
	@mkdir -p $(BUNDLE)
	@$(CXX) $< $(GUI_INCL) -o $(BUNDLE)/$@ $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(GUIFLAGS)
	@echo \ done.

install:
	@echo -n Install $(BUNDLE) to $(DESTDIR)$(LV2DIR)...
	@mkdir -p $(DESTDIR)$(LV2DIR)
	@rm -rf $(DESTDIR)$(LV2DIR)/$(BUNDLE)
	@cp -R $(BUNDLE) $(DESTDIR)$(LV2DIR)
	@echo \ done.

.PHONY: all

clean:
	@rm -rf $(BUNDLE)
