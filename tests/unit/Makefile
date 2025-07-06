CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

# Pfad zum Hauptverzeichnis
ROOT_DIR = ../..

# Test-Binaries
TEST_LAPTOP = test_laptop_operators
TEST_STORAGE = test_storage_drive_operators

# Standard-Ziel: Alle Tests bauen und ausführen
all: run_all

# Tests kompilieren
build_all: $(TEST_LAPTOP) $(TEST_STORAGE)

# Tests ausführen
run_all: build_all
	@echo "Running all unit tests..."
	@./$(TEST_LAPTOP)
	@echo ""
	@./$(TEST_STORAGE)

# Laptop-Operator-Tests kompilieren
$(TEST_LAPTOP): test_laptop_operators.cpp $(ROOT_DIR)/DataTypes.h $(ROOT_DIR)/debug_utils.h
	$(CXX) $(CXXFLAGS) -I$(ROOT_DIR) -o $@ $<

# Storage-Drive-Operator-Tests kompilieren
$(TEST_STORAGE): test_storage_drive_operators.cpp $(ROOT_DIR)/DataTypes.h $(ROOT_DIR)/debug_utils.h
	$(CXX) $(CXXFLAGS) -I$(ROOT_DIR) -o $@ $<

# Nur Laptop-Tests ausführen
run_laptop: $(TEST_LAPTOP)
	./$(TEST_LAPTOP)

# Nur Storage-Tests ausführen
run_storage: $(TEST_STORAGE)
	./$(TEST_STORAGE)

# Aufräumen
clean:
	rm -f $(TEST_LAPTOP) $(TEST_STORAGE) *.o

.PHONY: all clean build_all run_all run_laptop run_storage
