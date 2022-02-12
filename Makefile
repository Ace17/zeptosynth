BIN?=bin

all: true_all

CXXFLAGS+=-O3

PKGS:=\
  sdl2\
  alsa\

CXXFLAGS+=$(shell pkg-config $(PKGS) --cflags)
LDFLAGS+=$(shell pkg-config $(PKGS) --libs)

#------------------------------------------------------------------------------

ZEPTOSYNTH_SRCS:=\
  src/audio_output_sdl.cpp\
  src/main.cpp\
  src/midi_input.cpp\
  src/synth.cpp\
  src/osc.cpp\
  src/minblep.cpp\

TARGETS+=$(BIN)/zeptosynth.exe
$(BIN)/zeptosynth.exe: $(ZEPTOSYNTH_SRCS:%=$(BIN)/%.o)

#------------------------------------------------------------------------------

DEMO_SRCS:=\
  src/audio_output_sdl.cpp\
  src/main_demo.cpp\
  src/synth.cpp\
  src/osc.cpp\
  src/minblep.cpp\

TARGETS+=$(BIN)/demo.exe
$(BIN)/demo.exe: $(DEMO_SRCS:%=$(BIN)/%.o)

#------------------------------------------------------------------------------

PROFILE_SRCS:=\
  src/main_profile.cpp\
  src/synth.cpp\
  src/osc.cpp\
  src/minblep.cpp\

TARGETS+=$(BIN)/profile.exe
$(BIN)/profile.exe: $(PROFILE_SRCS:%=$(BIN)/%.o)

#------------------------------------------------------------------------------

true_all: $(TARGETS)

$(BIN)/%.exe:
	@mkdir -p $(dir $@)
	$(CXX) -o "$@" $^ $(LDFLAGS)

$(BIN)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o "$@" $<

clean:
	rm -rf $(BIN)

