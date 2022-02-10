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
  audio_output_sdl.cpp\
  main.cpp\
  midi_input.cpp\
  synth.cpp\

TARGETS+=$(BIN)/zeptosynth.exe
$(BIN)/zeptosynth.exe: $(ZEPTOSYNTH_SRCS:%=$(BIN)/%.o)

#------------------------------------------------------------------------------

DEMO_SRCS:=\
  audio_output_sdl.cpp\
  main_demo.cpp\
  synth.cpp\

TARGETS+=$(BIN)/demo.exe
$(BIN)/demo.exe: $(DEMO_SRCS:%=$(BIN)/%.o)

#------------------------------------------------------------------------------

PROFILE_SRCS:=\
  main_profile.cpp\
  synth.cpp\

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

