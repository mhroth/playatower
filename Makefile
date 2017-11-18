CC=clang
CXX=clang++
MKDIR=mkdir -p
SRCDIR=.
OUTDIR=.
BASEFLAGS=-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon -mtune=cortex-a7
CFLAGS=$(BASEFLAGS) -std=c11
CXXFLAGS=$(BASEFLAGS) -std=c++11 -fno-exceptions -fno-rtti
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
