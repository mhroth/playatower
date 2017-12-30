CC=clang
CXX=clang++
MKDIR=mkdir -p
SRCDIR=.
OUTDIR=.
BASEFLAGS=-O3 -DNDEBUG -ffast-math
ARCHFLAGS=-mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mtune=cortex-a53 # RPi3
# ARCHFLAGS=-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon -mtune=cortex-a7 # RPi2
CFLAGS=$(ARCHFLAGS) $(BASEFLAGS) -std=c11
CXXFLAGS=$(ARCHFLAGS) $(BASEFLAGS) -std=c++11 -fno-exceptions -fno-rtti
LIBFLAGS=-lpthread

HEADERS=$(wildcard $(SRCDIR)/*.h)
HEADERS+=$(wildcard $(SRCDIR)/*.hpp)

CFILES=$(wildcard $(SRCDIR)/*.c)
OBJC=$(CFILES:%.c=%.o)

CXXFILES=$(wildcard $(SRCDIR)/*.cpp)
OBJCXX=$(CXXFILES:%.cpp=%.o)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

all: vst2

vst2: $(OBJC) $(OBJCXX)
	$(CXX) -o $(OUTDIR)/playatower $^ $(LIBFLAGS)

.PHONY: clean

clean:
	rm -f $(SRCDIR)/*.o $(OUTDIR)/*.a $(OUTDIR)/playatower
