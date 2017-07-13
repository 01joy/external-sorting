CXXFLAGS=-std=c++11

SRC = sdk.cpp KMerge.cpp ProducerComsumer.cpp main.cpp
OBJS = $(SRC:.cpp=.o)
TARGET = sort.exe

all: $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET)

$(OBJS): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)
	 	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)