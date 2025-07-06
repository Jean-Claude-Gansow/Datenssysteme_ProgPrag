# Unit-Tests für Vergleichsoperatoren

Dieses Verzeichnis enthält Unit-Tests für die Vergleichsoperatoren in den Strukturen `laptop` und `storage_drive`.

## Inhalt

- `test_laptop_operators.cpp`: Tests für die Vergleichsoperatoren der `laptop`-Struktur
- `test_storage_drive_operators.cpp`: Tests für die Vergleichsoperatoren der `storage_drive`-Struktur
- `Makefile`: Build-System für die Tests

## Ausführung der Tests

### Alle Tests ausführen

```bash
cd tests/unit
make run_all
```

Oder verwenden Sie das Convenience-Skript:

```bash
cd tests
./run_operator_tests.sh
```

### Spezifische Tests ausführen

Nur Laptop-Tests:
```bash
cd tests/unit
make run_laptop
```

Nur Storage-Tests:
```bash
cd tests/unit
make run_storage
```

### Nur kompilieren (ohne Ausführung)

```bash
cd tests/unit
make build_all
```

### Aufräumen

```bash
cd tests/unit
make clean
```

## Testfälle

### Laptop Tests

1. `test_equals_identical`: Testet den `==` Operator bei identischen Laptops
2. `test_equals_different_critical`: Testet den `==` Operator bei Laptops mit unterschiedlichen kritischen Eigenschaften
3. `test_equals_some_matches`: Testet den `==` Operator bei Laptops mit einigen übereinstimmenden, aber nicht genügenden Eigenschaften
4. `test_equals_fallback_needed`: Testet den `==` Operator bei Laptops mit genügend übereinstimmenden Eigenschaften für Fallback
5. `test_similarity_operator`: Testet den `|` Operator
6. `test_index_operator`: Testet den `[]` Operator für den Zugriff auf Laptop-Eigenschaften
7. `test_null_values`: Testet den Vergleich mit NULL-Werten in bestimmten Feldern
8. `test_empty_laptops`: Testet den Vergleich von zwei vollständig leeren Laptop-Objekten

### Storage Drive Tests

1. `test_equals_identical`: Testet den `==` Operator bei identischen Storage-Drives
2. `test_equals_different_critical`: Testet den `==` Operator bei Storage-Drives mit unterschiedlichen kritischen Eigenschaften
3. `test_equals_some_matches`: Testet den `==` Operator bei Storage-Drives mit einigen übereinstimmenden, aber nicht genügenden Eigenschaften
4. `test_equals_fallback_needed`: Testet den `==` Operator bei Storage-Drives mit genügend übereinstimmenden Eigenschaften für Fallback
5. `test_similarity_operator`: Testet den `|` Operator
6. `test_index_operator`: Testet den `[]` Operator für den Zugriff auf Storage-Drive-Eigenschaften
7. `test_out_of_bounds_check`: Testet den Out-of-bounds Check in `storage_drive::operator==`
8. `test_null_values`: Testet den Vergleich mit NULL-Werten in bestimmten Feldern
9. `test_empty_drives`: Testet den Vergleich von zwei vollständig leeren Storage-Drive-Objekten
10. `test_specific_pattern_drives`: Testet den Vergleich von zwei Storage-Drives mit einem spezifischen Datenmuster [16 0 0 0 0 0 0 0 1 0 9 5]
