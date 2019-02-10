SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
SPU_DLL = spu.dll

XX_OBJS = $(SRCS:.cpp=.obj)
SPU_XX_DLL = spuxx.dll

DEBUG_FLAG = -g
CXX_DEBUG_FLAGS = $(DEBUG_FLAG)
CXX_PIC_FLAGS = -fPIC
CXXFLAGS = $(CXX_DEBUG_FLAGS) $(CXX_PIC_FLAGS) -w -I$(HCC1_SRCDIR)
CXXFLAGS_FOR_XX = $(CXX_DEBUG_FLAGS) $(CXX_PIC_FLAGS) -w -I$(HCXX1_SRCDIR) \
-DCXX_GENERATOR
RM = rm -r -f

all:$(SPU_DLL) $(SPU_XX_DLL)


$(SPU_DLL) : $(OBJS)
	$(CXX) $(DEBUG_FLAG) -shared -o $@ $(OBJS)

$(SPU_XX_DLL) : $(XX_OBJS)
	$(CXX) $(DEBUG_FLAG) -shared -o $@ $(XX_OBJS)

%.obj : %.cpp
	$(CXX) $(CXXFLAGS_FOR_XX) $< -o $@ -c

clean:
	$(RM) *.o *~ *.dll *.so .vs x64 Debug Release
	$(RM) *.obj
