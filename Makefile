CXXFLAGS= -std=c++11 -static -O3

SRC = sdk.cpp bounded_buffer.cpp loser_tree.cpp main.cpp
OBJS = $(SRC:.cpp=.o)
TARGET = sort.exe

all: $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET)

$(OBJS): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	 	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)