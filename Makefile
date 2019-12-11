CXX      = g++
CXXFLAGS = -Wall -Wextra -Werror -march=native -fpie -std=c++17 -g -O2
# LDFLAGS  = -lboost_system -lpthread
LDFLAGS  = -lpthread

LIB_NAME = libncomm.a

SRCS += source/channel.cpp
SRCS += source/network.cpp

OBJS = $(SRCS:.cpp=.o)

ifeq ($(DEBUG), 1)
	CXXFLAGS += -DNCOMM_PRINT
endif

default: $(OBJS)
	ar rcs $(LIB_NAME) $(OBJS)

test: default test/test-main.o
	$(CXX) $(CXXFLAGS) test/test-main.o test/tests.cpp -o run_test $(LDFLAGS) $(LIB_NAME)
	@./run_test

test-manual: default
	$(CXX) $(CXXFLAGS) test/test-manual.cpp -o test-manual $(LDFLAGS) $(LIB_NAME)

clean:
	find source/ -iname "*.o" -delete
	find test/ -iname "*.o" -delete
	rm -f $(LIB_NAME)

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

.PHONY: default clean test test-manual
