#include <type_traits>
#include <cstdint>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include "debug_utils.h"

#ifndef DUPLICATE_DETECTION_DATATYPES
#define DUPLICATE_DETECTION_DATATYPES



// --- print_helper für C++20: if constexpr & requires ---
template<typename T>
void print_helper(const T& obj) {
    if constexpr (requires { obj.println(); }) {
        obj.println();
    } else if constexpr (requires { obj.print(); }) {
        obj.print();
    } else if constexpr (std::is_pointer_v<T>) {
        printf("%p", static_cast<const void*>(obj));
    } else if constexpr (std::is_same_v<T, unsigned short>) {
        printf("%hu", obj);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        printf("%u", obj);
    } else if constexpr (std::is_same_v<T, int>) {
        printf("%d", obj);
    } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
        printf("%f", static_cast<double>(obj));
    } else {
        // nichts tun
    }
}



template <typename t>
struct dataSet
{
    t* data;
    size_t size;
    

    void print() const
    {
        for(unsigned long i = 0; i < size; i++)
        {
            printf("[%lu] := ", i);
            print_helper(data[i]);
            printf("\n");
        }
    }
    
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


    void println() const
    {
        for(unsigned int i = 0; i < N; i++)
        {
            printf("[%u] := ", i);
            print_helper(data[i]);
            printf("\n");
        }
    }

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
typedef unsigned short token;

typedef struct partition_t
{
    pair* data; // Pointer auf die Daten der Partition
    size_t size; // Größe der Partition
    size_t capacity;

    pair& operator[](unsigned int index) 
    {
        return data[index];
    }

    const pair& operator[](unsigned int index) const 
    {
        return data[index];
    }
} partition;

typedef struct matching_t
{
    match* matches;
    size_t size;

    match& operator[](unsigned int index) 
    {
        return matches[index];
    }

    const match& operator[](unsigned int index) const 
    {
        return matches[index];
    }
} matching;


typedef enum category_enum //define for laptop as well as storage_drive
{
    undef = -1,
    assembler_brand, //both
    assembler_modell, //both
    ram_capacity, storage_capacity = ram_capacity, //laptop , storage
    rom_capacity, connection_type = rom_capacity,//storage, storage
    cpu_brand, //laptop
    cpu_fam, //laptop
    cpu_series, //laptop
    gpu_brand, //laptop
    gpu_fam, //laptop
    gpu_series, //laptop
    display_resolution, //laptop
    display_size //laptop
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

    laptop()
    {
        memset(this, 0, sizeof(laptop));
    }

    token brand; //Index auf herstellerliste bzw Laptop marke
    token model; // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
    token rom;   // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
    token ram;   // Arbeitsspeicher in GB, z.B. 4, 8, 16, 32, 64
    token cpu_brand,cpu_fam,cpu_series; //store all information about the processor individually
    token gpu_brand, gpu_fam, gpu_series; // brand partially the same as cpu brand, familly and series should differ a bit
    token display_resolution; // Auflösung des Displays, z.B. 1920x1080, 2560x1440, 3840x2160
    token display_size; // Größe des Displays in Zoll, z.B. 13, 15, 17, 19


    size_t id;
    char * description; // Pointer auf eine Zeichenkette, die die Beschreibung des Laptops enthält

    void print() const
    {
        printf("[%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu]", brand, model, rom, ram, cpu_brand, cpu_fam, cpu_series, gpu_brand, gpu_fam, gpu_series, display_resolution, display_size);
    }

    void println() const
    {
        printf("[%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu]\n",brand,model,rom,ram,cpu_brand,cpu_fam,cpu_series,gpu_brand,gpu_fam,gpu_series,display_resolution,display_size);
    }

    const token &operator[](char idx) const
    {
        const token *tokens = &brand;
        return tokens[idx];
    }

    token &operator[](const int idx)
    {
        // Add bounds check to prevent out-of-bounds access        
        token *tokens = &brand;
        return tokens[idx];
    }

    token &operator[](const category idx)
    {
        // Add bounds check to prevent out-of-bounds access
        token *tokens = &brand;
        return tokens[idx];
    }

    token operator[](const category idx) const
    {
        const token *tokens = &brand; // erstes Token-Feld
        return tokens[idx];
    }

    int operator == (const laptop& other) const
    {
        //0 == nomatch
        //1 == ismatch
        //2 == use fallback if wanted
        const void* jumptable[] = {&&compFalse,&&compTrue};
        const void *jumptableTRUE[] = {
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&foundIdentical,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&useFallBack,
            &&useFallBack
        };
        const void *jumptableUNKNOWN[] =
        {
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&useFallBack,
            &&useFallBack
        };
        char c = assembler_brand; // Start with first category
        char equal = 0;
        const char required = 3;        
        
        checkComp:
        // Check if we've gone beyond the valid categories                  
        if(!(*this)[c] || !other[c])
        {
            //DEBUG_MATCH("Category %d (%s) not comparable (values: %hu, %hu)\n", c, category_name(c), (*this)[c], other[c]);
            goto notComparable;
        }
        
        goto *jumptable[(*this)[c] == other[c]]; //returns 0 or 1, 0 should be false, 1 should be true

        compTrue:
        ++equal;
        ++c;
        
        // Check for out-of-bounds jump in jumptableRET        
        goto *jumptableTRUE[equal];
        
        compFalse:
        return 0; //0 == not identical, found information discarding equality
        
        notComparable:
        ++c;
        
        // prevent going out of bounds, by limiting where to jump to    
        goto *jumptableUNKNOWN[c];

        useFallBack:
        if(equal > required) //if we found enough to suspect a match, look again with a different method
        {
            return 2;
        }
        return 0; //otherwise dont waste time looking

        foundIdentical:
        this->print();
        printf(" == ");
        other.println();
        return 1;
    }

    double operator | (const laptop& other) const  //implementiere eine Vergleichsfunktion basierend auf djakar
    {
        /*void* jumpTable1[128] = {};
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
        //for(char* c1 = this->description,*c2 = other.description;;)
        //{
            
        //} 
        return 0.0; // Beispiel: 0.0 für gleiche IDs, 1.0 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten*/
        return 0.0; //noch nicht implemenmtiert, beraten was die beste methode ist, ich schlage 2er-tupel jaccar vor, da es sehr effizent umsetzbar ist
    }
}; 

struct storage_drive
{
    storage_drive()
    {
        memset(this,0,sizeof(storage_drive));
    }

    token brand;                          // Index auf herstellerliste bzw Laptop marke
    token model;                          // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
    token rom;                            // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
    token connection_type;

    size_t id;
    quintupel* descriptor;

    void print() const
    {
        printf("[%hu %hu %hu]", brand, model, rom);
    }

    void println() const
    {
        printf("[%hu %hu %hu]\n", brand, model, rom);
    }

    const token &operator[](char idx) const
    {
        const token *tokens = &brand;
        return tokens[idx];
    }

    token &operator[](const char idx)
    {
        token *tokens = &brand;
        return tokens[idx];
    }

    token &operator[](const category idx)
    {
        token *tokens = &brand;
        return tokens[idx];
    }

    token operator[](const category idx) const
    {
        const token *tokens = &brand; // erstes Token-Feld
        return tokens[idx];
    }

    int operator==(storage_drive &other)
    {
        return (*this == static_cast<const storage_drive&>(other));
    }

    int operator==(const storage_drive &other) const
    {
        //0 == nomatch
        //1 == ismatch
        //2 == use fallback if wanted
        const void* jumptable[] = {&&compFalse, &&compTrue};
        const void *jumptableTRUE[] = {
            &&checkComp, // increase amount of checkComp Lables to (variable 18 lines down)required whenever we add a field
            &&checkComp,
            &&foundIdentical, //2 equal = match
            &&useFallBack, //not really needed, just to make sure we dont run over the edge (should not happen)
            &&useFallBack
        };
        const void *jumptableUNKNOWN[] =
        {
            &&checkComp,
            &&checkComp,
            &&checkComp,
            &&useFallBack,
            &&useFallBack
        };
        
        char c = assembler_brand;  // Start with brand comparison
        char equal = 0;
        const char required = 2;   // Need at least 2 matching categories
        
        checkComp:               
        if(!(*this)[c] || !other[c])
        {
            //DEBUG_MATCH("Storage category %d (%s) not comparable (values: %hu, %hu)\n", c, category_name(c), (*this)[c], other[c]);
            goto notComparable;
        }
        
        goto *jumptable[(*this)[c] == other[c]];

        compTrue:
        ++equal;
        ++c;
        
        // Jump to the next step based on how many matches we've found        
        goto *jumptableTRUE[equal];
        
        compFalse:
        return 0; //0 == not identical, found information discarding equality
        
        notComparable:
        ++c;
        // Jump to next step, let jumptable handle bounds    
        goto *jumptableUNKNOWN[c];

        useFallBack:
        if(equal > required) //if we found enough to suspect a match, look again with a different method
        {
            return 2;
        }
        return 0; //otherwise dont waste time looking

        foundIdentical:
        this->print(); //just for debug, take out when not needed anylonger
        printf(" == ");
        other.println();
        return 1;
    }

    double operator|(const storage_drive &other) const // implementiere eine Vergleichsfunktion basieren auf einer anderen Metrik als dem Direkten tokenvergleich.
    {

        return 0.0; // Beispiel: 0.0 für gleiche IDs, 1.0 für unterschiedliche -- berechne basierend auf ähnlichkeitswerten
    }
};

#endif