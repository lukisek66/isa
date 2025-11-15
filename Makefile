CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

SRC := src/main.cpp src/filter.cpp src/dns_server.cpp src/forwarder.cpp
OBJ := src/main.o src/filter.o src/dns_server.o src/forwarder.o
TARGET := dns

all: $(TARGET)

# Linkování
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

# Překlad jednotlivých .cpp souborů
src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/filter.o: src/filter.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/dns_server.o: src/dns_server.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/forwarder.o: src/forwarder.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) -s 1.1.1.1 -f blocked.txt -v

clean:
	rm -f $(OBJ) $(TARGET)
