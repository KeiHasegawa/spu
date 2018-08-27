SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
SPU_DLL = spu.dll

DEBUG_FLAG = -g
CXX_DEBUG_FLAGS = $(DEBUG_FLAG)
CXX_PIC_FLAGS = -fPIC
CXXFLAGS = $(CXX_DEBUG_FLAGS) $(CXX_PIC_FLAGS) -w -I$(HCC1_SRCDIR)
RM = rm -r -f

all:$(SPU_DLL)


$(SPU_DLL) : $(OBJS)
	$(CXX) $(CXX_DEBUG_FLAGS) -shared -o $@ $(OBJS)

clean:
	$(RM) *.o *~ *.dll *.so .vs x64 Debug Release
