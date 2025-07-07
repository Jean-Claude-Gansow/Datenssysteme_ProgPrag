#!/bin/bash

# Farben für die Ausgabe
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

cd "$(dirname "$0")"

echo -e "${YELLOW}=== Kompiliere Memory Check für Matching Manager ===${NC}"

# Kompiliere mit Debug-Informationen und ohne Optimierung
g++ -std=c++20 -g -O0 memory_check.cpp -o memory_check

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Kompilierung erfolgreich${NC}"
    
    echo -e "${YELLOW}\n=== Führe Memory Check aus ===${NC}"
    
    # Optional: Mit Valgrind ausführen, wenn verfügbar
    if command -v valgrind &>/dev/null; then
        echo -e "${YELLOW}Valgrind gefunden, führe Speicherprüfung durch...${NC}"
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./memory_check
    else
        echo -e "${YELLOW}Valgrind nicht gefunden, führe normalen Test durch...${NC}"
        ./memory_check
    fi
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}\nMemory Check erfolgreich abgeschlossen!${NC}"
        exit 0
    else
        echo -e "${RED}\nMemory Check fehlgeschlagen!${NC}"
        exit 1
    fi
else
    echo -e "${RED}Kompilierung fehlgeschlagen${NC}"
    exit 1
fi
