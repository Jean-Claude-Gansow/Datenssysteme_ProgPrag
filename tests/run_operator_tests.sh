#!/bin/bash

# Wechsle ins Unit-Test-Verzeichnis
cd "$(dirname "$0")/unit"

# Ausführen des Makefiles mit "all" als Ziel
make all

# Exit-Code des make-Befehls übernehmen
exit $?
