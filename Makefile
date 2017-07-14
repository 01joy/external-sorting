CXXFLAGS=-std=c++11

SRC = sdk.cpp bounded_buffer.cpp KMerge.cpp main.cpp
OBJS = $(SRC:.cpp=.o)
TARGET = sort.exe

all: $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET)

$(OBJS): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	 	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)