CXX      = g++
CXXFLAGS = -Wall -Wextra -Werror -march=native -fpie -g -O2
LDFLAGS  = -lboost_system -lpthread

LIB_NAME = ncomm.a

SRCS += source/channel.cpp
SRCS += source/network.cpp

OBJS = $(SRCS:.cpp=.o)

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DNCOMM_PRINT
endif

default: $(OBJS)
	ar rcs $(LIB_NAME) $(OBJS)

clean:
	find source/ -iname "*.o" -delete
	rm -f $(LIB_NAME)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

.PHONY: default clean
