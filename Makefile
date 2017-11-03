CC=clang
CXX=clang++
MKDIR=mkdir -p
SRCDIR=.
OUTDIR=.
BASEFLAGS=
CFLAGS=$(BASEFLAGS) -std=c11
CXXFLAGS=$(BASEFLAGS) -std=c++11 -fno-exceptions -fno-rtti

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
	$(CXX) -o $(OUTDIR)/playatower $^

.PHONY: clean

clean:
	rm -f $(SRCDIR)/*.o $(OUTDIR)/*.a $(OUTDIR)/playatower
