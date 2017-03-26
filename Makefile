SRC = KMerge.cpp ProducerComsumer.cpp Sort.cpp
OBJS = $(SRC:.cpp=.o)
TARGET = Sort.exe

all: $(TARGET)

clean:
	-rm -f $(OBJS) $(TARGET)

$(OBJS): $(SRC)
	$(CXX) -c $(SRC)
	 	
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)