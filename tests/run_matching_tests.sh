#!/bin/bash

# Farben für die Ausgabe
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Kompiliere Matching Manager Test ===${NC}"

# Wechsle ins Verzeichnis mit den Tests
cd "$(dirname "$0")/unit"

# Kompiliere den Test
g++ -std=c++20 -g -O0 test_matching_mgr.cpp -o test_matching_mgr -I../../

# Prüfe, ob die Kompilierung erfolgreich war
if [ $? -eq 0 ]; then
    echo -e "${GREEN}Kompilierung erfolgreich!${NC}"

    echo -e "${YELLOW}\n=== Führe Tests aus ===${NC}"
    
    # Führe den Test aus
    ./test_matching_mgr
    
    # Prüfe, ob der Test erfolgreich war
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}\n=== Tests erfolgreich abgeschlossen! ===${NC}"
    else
        echo -e "${RED}\n=== Tests fehlgeschlagen! ===${NC}"
        exit 1
    fi
    
else
    echo -e "${RED}Kompilierung fehlgeschlagen!${NC}"
    exit 1
fi

exit 0
