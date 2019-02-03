INC=-I/usr/lib/llvm-6.0/include/
all: LocalOpts.so

CXX = clang
CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) $(INC) -fPIC -g -O0

%.so: %.o
	$(CXX) -dlyb -shared $^ -o $@

clean: 
	rm -f *.o *~ *.so
