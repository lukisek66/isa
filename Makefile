# Kompilátor a příznaky
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

# Soubory projektu
SRC := src/main.cpp src/filter.cpp src/dns_server.cpp src/forwarder.cpp
OBJ := $(SRC:.cpp=.o)
TARGET := dns

# Výchozí cíl
all: $(TARGET)

# Linkování výsledného binárního souboru
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Překlad jednotlivých .cpp souborů
%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Spuštění programu s ukázkovými parametry
run: $(TARGET)
	./$(TARGET) -s 1.1.1.1 -f blocked.txt -v

# (Volitelně) spustí testy, až je vytvoříš
test: $(TARGET) $(TEST_FILTER)
	@echo "==> Spouštím testy..."
	bash tests/test_main.sh
	bash tests/test_dns_server.sh

# Úklid po překladu
clean:
	rm -f $(OBJ) $(TARGET)
