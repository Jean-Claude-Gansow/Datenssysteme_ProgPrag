#ifndef DUPLICATE_DETECTION_DATATYPES
#define DUPLICATE_DETECTION_DATATYPES

#include <cstdint>

template <typename t>
struct dataSet
{
    t* data;
    unsigned long size;
    const t operator[](unsigned int index) const 
    {
        return data[index];
    }
    t* c_arr()
    {
        return data;
    }
};

template <unsigned int N,typename t>
struct tuple_t {
    t data[N];

    // Optional: Zugriffsfunktion
    t& operator[](unsigned int index) 
    {
        return data[index];
    }

    const t& operator[](unsigned int index) const 
    {
        return data[index];
    }
};

typedef tuple_t<1,uintptr_t> single_t;
typedef tuple_t<5,uintptr_t> storage;
typedef tuple_t<2,uintptr_t> pair,match;
typedef pair** block_t;

typedef struct matching_t
{
    match* matches;
    unsigned int size;

    match& operator[](unsigned int index) 
    {
        return matches[index];
    }

    const match& operator[](unsigned int index) const 
    {
        return matches[index];
    }
} matching;

typedef struct Laptop
{ 
    /*char marke; //bzw Hersteller
    char modell; //index auf ModellListe modellliste pro marke bzw Hersteller
    short cpu; //aufteilen in Familie und serie, wenn möglich 4 bit für Familie und 12 bit für Serie da es deutlich mehr Serien als Familien gibt, überarbeiten falls das den Ramen sprengt
    char ram; //bzw Arbeitsspeicher --> all gb notes <= 64gb once filled, dont look further
    char rom; //bzw Speicherkapazität --> all gb notes > 64gb
    short gpu; //aufteilen in Marke, modell und Speicherkapazität, wenn möglich 
    char display-resolution;
    char display-size;*/
    union 
    {
        struct 
        {
            char model; // Index auf ModellListe, Modellliste pro Marke bzw. Hersteller
            short cpu; // Aufteilen in Familie und Serie, wenn möglich 4 Bit für Familie und 12 Bit für Serie da es deutlich mehr Serien als Familien gibt, überarbeiten falls das den Rahmen sprengt
            char ram; // Arbeitsspeicher in GB, z.B. 4, 8, 16, 32, 64
            char rom; // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048
            short gpu; // Aufteilen in Marke, Modell und Speicherkapazität, wenn möglich
            char display_resolution; // Auflösung des Displays, z.B. 1920x1080, 2560x1440, 3840x2160
            char display_size; // Größe des Displays in Zoll, z.B. 13, 15, 17, 19
        };
    }; // Union für verschiedene Attribute des
    
    char * description; // Pointer auf eine Zeichenkette, die die Beschreibung des Laptops enthält
    
    const bool operator == (const Laptop& other) const 
    {
        return (*this & other) == *this; // Beispiel: 0 für gleiche IDs, 1 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten
    }

    const double operator | (const Laptop& other) const 
    {
        return strncmp(description,other.description); // Beispiel: 0.0 für gleiche IDs, 1.0 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten
    }
};

#endif