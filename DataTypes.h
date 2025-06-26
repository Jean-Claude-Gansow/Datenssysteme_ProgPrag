#ifndef DUPLICATE_DETECTION_DATATYPES
#define DUPLICATE_DETECTION_DATATYPES

#include <cstdint>
#include <cstring>

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
typedef tuple_t<5,uintptr_t> quintupel;
typedef tuple_t<2,uintptr_t> pair,match;
typedef pair** block_t;
typedef char token;

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


typedef enum category_enum
{
    undef = -1,
    assembler_brand,
    assembler_modell,
    ram_capacity,
    rom_capacity,
    cpu_brand,
    cpu_fam,
    cpu_series,
    gpu_brand,
    gpu_fam,
    gpu_series,
    display_resolution,
    display_size
} category, token_class;

struct laptop
{ 
    /*char marke; //bzw Hersteller
    char modell; //index auf ModellListe modellliste pro marke bzw Hersteller
    short cpu; //aufteilen in Familie und serie, wenn möglich 4 bit für Familie und 12 bit für Serie da es deutlich mehr Serien als Familien gibt, überarbeiten falls das den Ramen sprengt
    char ram; //bzw Arbeitsspeicher --> all gb notes <= 64gb once filled, dont look further
    char rom; //bzw Speicherkapazität --> all gb notes > 64gb
    short gpu; //aufteilen in Marke, modell und Speicherkapazität, wenn möglich 
    char display-resolution;
    char display-
    size;*/
    union obj_token 
    {
        struct token_classes
        {
            token brand; //Index auf herstellerliste bzw Laptop marke
            token model; // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
            token rom;   // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
            token ram;   // Arbeitsspeicher in GB, z.B. 4, 8, 16, 32, 64
            token cpu_brand,cpu_fam,cpu_series; //store all information about the processor individually
            token gpu_brand, gpu_fam, gpu_series; // brand partially the same as cpu brand, familly and series should differ a bit
            token display_resolution; // Auflösung des Displays, z.B. 1920x1080, 2560x1440, 3840x2160
            token display_size; // Größe des Displays in Zoll, z.B. 13, 15, 17, 19
        };
    }; // Union für verschiedene Attribute des
    
    char * description; // Pointer auf eine Zeichenkette, die die Beschreibung des Laptops enthält
    
    bool operator == (const laptop& other) const 
    {
        return (*this| other) == 1.0;
    }

    double operator | (const laptop& other) const  //implementiere eine Vergleichsfunktion basierend auf djakar
    {
        void* jumpTable1[128] = {};
        void *jumpTable2[128] = {};
        char alphabetThis[127];
        char alphabetOther[127];
        memset(alphabetThis, 0, 127);
        memset(alphabetOther, 0, 127);
        char* c1 = this->description-1,*c2 =other.description-1; //wir fangen mit ++c1 an, d.h. wir landen bei 

        loopc1:
        alphabetThis[*c1]++;
        goto *jumpTable1[*(++c1)];
        loopc2:
        alphabetOther[*c2]++;
        goto *jumpTable2[*(++c2)];
        endLoopC1:
        jumpTable2[0] = &&exitUs;
        c1 = c2; //switch lable 2
        goto *jumpTable1[*c2];
        endLoopC2: 
        jumpTable1[0] = &&exitUs;
        c2 = c1;
        goto *jumpTable2[*c2];
        exitUs:
        //hier methode zum ähnlichkeitsindex basierend auf buchstabencount nutzen (experimentell, weitere operatoren definieren)
        
        //alphabet[32] = 0;
        for(char* c1 = this->description,*c2 = other.description;;)
        {
            
        } 
        return 0.0; // Beispiel: 0.0 für gleiche IDs, 1.0 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten
    }
}; 

struct storage_drive
{
    storage_drive()
    {
        memset(this,0,sizeof(storage_drive));
    }

    union
    {
        struct
        {
            token brand;                          // Index auf herstellerliste bzw Laptop marke
            token model;                          // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
            token rom;                            // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
            token fill;
        };
    }; // Union für verschiedene Attribute des

    quintupel* descriptor;

    bool operator==(const laptop &other) const
    {
        return (*this | other) == 1.0;
    }

    double operator|(const laptop &other) const // implementiere eine Vergleichsfunktion basieren auf einer anderen Metrik als dem Direkten tokenvergleich.
    {

        return 0.0; // Beispiel: 0.0 für gleiche IDs, 1.0 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten
    }
};

#endif