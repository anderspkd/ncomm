CXX      = g++
CXXFLAGS = -Wall -Wextra -Werror -march=native -fpie
LDFLAGS  = -lboost_system -lpthread

LIB_NAME = ncomm.a

SRCS  = src/dummy.cpp
SRCS += src/asio.cpp

OBJS = $(SRCS:.cpp=.o)

default: $(OBJS)
	ar rcs $(LIB_NAME) $(OBJS)

test_dummy:
	$(CXX) $(CXXFLAGS) test_dummy.cpp -o test_dummy $(LIB_NAME) -lpthread

test_asio:
	$(CXX) $(CXXFLAGS) test_asio.cpp -o test_asio $(LIB_NAME) -lpthread

clean:
	find src/ -iname "*.o" -delete
	rm -f $(LIB_NAME)
	rm -f test_dummy
	rm -f test_asio

.SUFFIXES: .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(LDFLAGS)

.PHONY: default test_dummy test_asio clean
