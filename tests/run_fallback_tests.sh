#!/bin/bash
# filepath: /mnt/h/Documents/Technische_Universität/Datensysteme Praktikum/Datenssysteme_ProgPrag/tests/run_fallback_test.sh

# Farben für bessere Lesbarkeit
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Banner
echo -e "${BLUE}=======================================${NC}"
echo -e "${BLUE}   Fallback-Test Diagnose-Tool v1.0    ${NC}"
echo -e "${BLUE}=======================================${NC}"

# Pfade festlegen
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
TEST_FILE="$SCRIPT_DIR/unit/fallback_test.cpp"
OUTPUT_BIN="$SCRIPT_DIR/unit/fallback_test"

# Prüfen, ob Test existiert
if [ ! -f "$TEST_FILE" ]; then
    echo -e "${RED}Fehler: Test-Datei nicht gefunden: $TEST_FILE${NC}"
    exit 1
fi

# Kompilieren
echo -e "\n${YELLOW}Kompiliere Fallback-Test...${NC}"
g++ -std=c++17 -g -Wall -Wextra "$TEST_FILE" -o "$OUTPUT_BIN" -I"$PROJECT_ROOT" -pthread

if [ $? -ne 0 ]; then
    echo -e "${RED}Kompilierung fehlgeschlagen!${NC}"
    exit 1
fi
echo -e "${GREEN}Kompilierung erfolgreich.${NC}"

# Test ausführen
echo -e "\n${YELLOW}Führe Fallback-Test aus...${NC}"
"$OUTPUT_BIN"

# Exit-Code prüfen
TEST_RESULT=$?
if [ $TEST_RESULT -ne 0 ]; then
    echo -e "\n${RED}Test fehlgeschlagen mit Fehlercode $TEST_RESULT${NC}"
else
    echo -e "\n${GREEN}Test erfolgreich abgeschlossen.${NC}"
fi

# Analyse der Fallback-Probleme
echo -e "\n${BLUE}Diagnose möglicher Probleme:${NC}"

# 1. Prüfe operator== Implementation
echo -e "\n${YELLOW}1. Prüfe operator== Implementation${NC}"
grep -n "useFallBack:" "$PROJECT_ROOT/DataTypes.h"
grep -n "return 2" "$PROJECT_ROOT/DataTypes.h" --color=auto

# 2. Prüfe Jaccard-Implementation
echo -e "\n${YELLOW}2. Prüfe Jaccard-Implementation${NC}"
grep -n "operator|" "$PROJECT_ROOT/DataTypes.h" --color=auto

# 3. Prüfe Fallback-Code im Matching Manager
echo -e "\n${YELLOW}3. Prüfe Fallback-Code im Matching Manager${NC}"
grep -n "fallback:" "$PROJECT_ROOT/Matching_mngr.h" --color=auto
grep -n "similarity =" "$PROJECT_ROOT/Matching_mngr.h" --color=auto

# Optionen anbieten
echo -e "\n${BLUE}Weitere Diagnose-Optionen:${NC}"
echo "1. Test mit Valgrind auf Speicherlecks prüfen"
echo "2. Test mit Detailausgabe ausführen"
echo "3. Ähnlichkeitsschwellwert temporär senken (für Debugging)"
echo "4. Beenden"

read -p "Auswahl: " choice

case $choice in
    1)
        echo -e "\n${YELLOW}Führe Test mit Valgrind aus...${NC}"
        valgrind --leak-check=full --show-leak-kinds=all "$OUTPUT_BIN"
        ;;
    2)
        echo -e "\n${YELLOW}Führe Test mit Detailausgabe aus...${NC}"
        DEBUG_LEVEL=3 "$OUTPUT_BIN"
        ;;
    3)
        echo -e "\n${YELLOW}Modifiziere temporär den Ähnlichkeitsschwellwert...${NC}"
        # Erstelle Backup
        cp "$PROJECT_ROOT/Matching_mngr.h" "$PROJECT_ROOT/Matching_mngr.h.bak"
        # Ändere den Schwellwert von 0.85 auf 0.5
        sed -i 's/if (similarity >= 0.85)/if (similarity >= 0.50)/' "$PROJECT_ROOT/Matching_mngr.h"
        
        echo -e "${YELLOW}Kompiliere mit geändertem Schwellwert...${NC}"
        g++ -std=c++17 -g -Wall -Wextra "$TEST_FILE" -o "${OUTPUT_BIN}_modified" -I"$PROJECT_ROOT" -pthread
        
        echo -e "${YELLOW}Führe modifizierten Test aus...${NC}"
        "${OUTPUT_BIN}_modified"
        
        # Stelle Original wieder her
        mv "$PROJECT_ROOT/Matching_mngr.h.bak" "$PROJECT_ROOT/Matching_mngr.h"
        echo -e "${GREEN}Original-Datei wiederhergestellt.${NC}"
        ;;
    *)
        echo -e "\n${BLUE}Diagnose beendet.${NC}"
        ;;
esac

# Ergebnis-Zusammenfassung
echo -e "\n${BLUE}=======================================${NC}"
echo -e "${YELLOW}Fallback-Test Diagnose abgeschlossen${NC}"
echo -e "${BLUE}=======================================${NC}"
echo -e "Überprüfen Sie die Testergebnisse und vergleichen Sie mit den erwarteten Werten."
echo -e "Wenn keine Fallback-Ausgaben erscheinen, prüfen Sie die folgenden Punkte:"
echo -e "1. Gibt der operator== in DataTypes.h tatsächlich den Wert 2 zurück?"
echo -e "2. Ist die Jaccard-Ähnlichkeit hoch genug (>= 0.85)?"
echo -e "3. Wird der Fallback-Code überhaupt erreicht (goto *jumpTable[result])?"
echo -e "4. Sind die Beschreibungstexte der Einträge korrekt formatiert?"
echo -e "${BLUE}=======================================${NC}"

exit $TEST_RESULT