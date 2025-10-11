# Kompilátor a příznaky
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

# Soubory projektu
SRC := src/main.cpp src/filter.cpp src/dns_server.cpp
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
	./$(TARGET) -s 8.8.8.8 -f blocked.txt -v

# (Volitelně) spustí testy, až je vytvoříš
test: $(TARGET)
	@echo "==> Spouštím testy..."
	@./$(TARGET) -s 8.8.8.8 -f blocked.txt -v

# Úklid po překladu
clean:
	rm -f $(OBJ) $(TARGET)
