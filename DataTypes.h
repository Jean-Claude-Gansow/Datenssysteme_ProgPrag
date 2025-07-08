#include <type_traits>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <stdlib.h>
#include <cstdio>
#include <set>
#include <unordered_set>

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

    t operator[](unsigned int index) 
    {
        return data[index];
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

typedef struct match_t
{
    uintptr_t data[2];
    double jaccard_index = 0.0;
    
}match;

typedef tuple_t<1,uintptr_t> single_t;
typedef tuple_t<5,uintptr_t> quintupel;
typedef tuple_t<2,uintptr_t> pair;
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
    rom_capacity, class_a = rom_capacity,//storage, storage
    cpu_brand, class_c = cpu_brand,  //laptop
    cpu_fam, class_v = cpu_fam,//laptop
    cpu_series, class_u = cpu_series, //laptop
    gpu_brand, class_uhs = gpu_brand, //laptop
    gpu_fam,  variant = gpu_fam,//laptop
    gpu_series,  data_speed = gpu_series,//laptop
    display_resolution, formfactor = display_resolution, //laptop
    display_size, connection_type = display_size //laptop
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
        token_count = 0;
    }

    token brand; //Index auf herstellerliste bzw Laptop marke
    token model; // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
    token rom;   // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
    token ram;   // Arbeitsspeicher in GB, z.B. 4, 8, 16, 32, 64
    token cpu_brand,cpu_fam,cpu_series; //store all information about the processor individually
    token gpu_brand, gpu_fam, gpu_series; // brand partially the same as cpu brand, familly and series should differ a bit
    token display_resolution; // Auflösung des Displays, z.B. 1920x1080, 2560x1440, 3840x2160
    token display_size; // Größe des Displays in Zoll, z.B. 13, 15, 17, 19

    size_t token_count; // Anzahl der Tokens mit Informationen (> 0)
    size_t id;
    single_t* descriptor; // Pointer auf eine Zeichenkette, die die Beschreibung des Laptops enthält
    uint32_t* numeral_buffer; //dont forget to link this before comparing
    uint32_t numNumerals = 0;

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

    std::unordered_set<uint32_t>* generate_set() {
        // Stelle sicher, dass der numeral_buffer korrekt verlinkt ist
        auto processString = [&](char *str, laptop *obj)
        {
            char *p = str;
            int tokenValue = 0;

            const void *jumpTableLoop[256] =
                {
                    &&eol, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 0–7
                    &&stn, &&stn, &&eol, &&stn, &&stn, &&stn, &&stn, &&stn,             // 8–15
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 16–23
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 24–31
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 32–39
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 40–47
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 48–55
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 56–63
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 64–71
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 72–79
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 80–87
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 88–95
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 96–103
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 104–111
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 112–119
                    &&stn, &&stn, &&stn, &&stn, &&loop_whitespace, &&stn, &&stn, &&stn, // 120–127
                };

            const void *jumpTableFound8[256] =
                {
                    &&sChar, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 0–7
                    &&gt1char, &&gt1char, &&sChar, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 8–15
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 16–23
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 24–31
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 32–39
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 40–47
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 48–55
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 56–63
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 64–71
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 72–79
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 80–87
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 88–95
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 96–103
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 104–111
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 112–119
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&sChar, &&gt1char, &&gt1char, &&gt1char,   // 120–127
                };

            const void *jumpTableFound16[256] =
                {
                    &&sShort, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char,  // 0–7
                    &&ge2char, &&ge2char, &&sShort, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char,  // 8–15
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 16–23
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 24–31
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 32–39
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 40–47
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 48–55
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 56–63
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 64–71
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 72–79
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 80–87
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 88–95
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 96–103
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 104–111
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 112–119
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&sShort, &&ge2char, &&ge2char, &&ge2char, // 120–127
                };

            const void *jumpTableFound24[256] =
                {
                    &&ssShort, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, // 0–7
                    &&sIntL, &&sIntL, &&ssShort, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, // 8–15
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 16–23
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 24–31
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 32–39
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 40–47
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 48–55
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 56–63
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 64–71
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 72–79
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 80–87
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 88–95
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 96–103
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 104–111
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 112–119
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&ssShort, &&sIntL, &&sIntL, &&sIntL, // 120–127
                };

            const void *jumpTableIntLoop[256] =
                {
                    &&eol, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,             // 0–7
                    &&sIntL, &&sIntL, &&eol, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,             // 8–15
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 16–23
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 24–31
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 32–39
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 40–47
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 48–55
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 56–63
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 64–71
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 72–79
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 80–87
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 88–95
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 96–103
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 104–111
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 112–119
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&loop_whitespace, &&sIntL, &&sIntL, &&sIntL, // 120–127
                };
            
            goto *jumpTableLoop[*p];
            uint32_t numeral;
                //_|_|_|c-c-c-c ... |

            stn :
                // save current destination
                char *pos = p;
                char roffset = 0;
                goto *jumpTableFound8[*(++p)]; // if the next char after dest is a whitespace, jump to save char, to only save one char in a number
            gt1char: //otherwise we will land here
                goto *jumpTableFound16[*(++p)]; //if the 2nd byte after dst is a whitespace, jump to sShort for we got at least 2 letters we will take as 1 int with 16bit offset
            ge2char: //otherwise we will land here
                goto *jumpTableFound24[*(++p)]; //if the 3rd byte after dst is a whitespace, jump to ssShort for we got 3 letters we will take as 1 int with 8 bit offset
            sIntL: //otherwise we got 4 or mehr letters in a row, therefor we loop_saveInt
                numeral = *((uint32_t*)pos); //get numeral at pos and increment pos for next possible step to be in a different position
                pos++;
                obj->numeral_buffer[obj->numNumerals] = numeral; //save numeral to buffer
                obj->numNumerals++; //increment number of numerals for correct saving location and later work
                goto *jumpTableIntLoop[*(++p)]; //if the next byte is also not a whitespace, will land back here, otherwise will loop whitespace 
            
            sChar:
                roffset = 24; //have a 24 offset ready for shift, and jump to save as integer;
                goto sInt;
            sShort: //save 2 byte in an integer by offsetting the current data >> 16
                roffset = 16;
                goto sInt;
            ssShort: //save 3 byte in an int by offsetting the current data >> 8
                roffset = 8;
                goto sInt;
            sInt:
                numeral = *((uint32_t*)pos);
                uint32_t onumeral = numeral >> roffset; //get data at destination for exactly the filed bytes we need
                obj->numeral_buffer[obj->numNumerals] = (uint32_t)onumeral;  //save the shifted numeral 
                obj->numNumerals++;// increase for correct save location
                goto *jumpTableLoop[*p];
                                
            loop_whitespace:
                ++p;
                goto *jumpTableLoop[*p];

            eol:
                return;
        };

        // Zuerst den String aus Index 0 verarbeiten
        numNumerals = 0; // Sicherstellen, dass wir bei 0 anfangen
        processString(reinterpret_cast<char *>(this->descriptor->data[0]), this);
        
        // Erstelle ein Set aus dem numeral_buffer für den ersten String
        std::unordered_set<uint32_t>* combined_set = new std::unordered_set<uint32_t>(numeral_buffer, numeral_buffer + numNumerals);

        return combined_set;
    }
};

struct storage_drive
{
    // Hilfsfunktion zum sicheren Schreiben in numeral_buffer
    static void safe_write_numeral(uint32_t* buffer, uint32_t& numNumerals, uint32_t value) {
        if (buffer && numNumerals < 500) { // 500 ist die angenommene Puffergröße
            buffer[numNumerals++] = value;
        } else {
            printf("WARNING: Buffer overflow prevented (numNumerals=%u)\n", numNumerals);
        }
    }

    storage_drive()
    {
        memset(this,0,sizeof(storage_drive));
        token_count = 0;
    }

    token brand = 0;                          // Index auf herstellerliste bzw Laptop marke
    token model = 0;                          // Index auf ModellListe bzw Laptop Serienname eg ThinkPad
    token capacity = 0;                            // Speicherkapazität in GB, z.B. 128, 256, 512, 1024, 2048 or TB, z.b. 1TB 2TB 3TB...
    token class_a = 0;
    token class_c = 0;
    token class_v = 0;
    token class_u = 0;
    token class_uhs = 0;
    token variant = 0;
    token data_speed = 0;
    token formfactor = 0;
    token connection_type = 0;               

    size_t token_count = 0;                   // Anzahl der Tokens mit Informationen (> 0)
    size_t id = 0;
    quintupel* descriptor = nullptr;
    uint32_t *numeral_buffer; // dont forget to link this before comparing
    uint32_t numNumerals = 0;

    void print() const
    {
        printf("[%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu]", brand, model, capacity, class_a, class_c, class_v, class_u, class_uhs, variant ,data_speed, formfactor, connection_type);
    }

    void println() const
    {
        printf("[%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu]\n", brand, model, capacity, class_a, class_c, class_v, class_u, class_uhs, variant, data_speed, formfactor, connection_type);
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

    std::unordered_set<uint32_t> *generate_set()
    {
        // Stelle sicher, dass der numeral_buffer korrekt verlinkt ist
        auto processString = [&](char *str, storage_drive *obj)
        {
            char *p = str;
            int tokenValue = 0;
            const void *jumpTableLoop[256] =
                {
                    &&eol, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 0–7
                    &&stn, &&stn, &&eol, &&stn, &&stn, &&stn, &&stn, &&stn,             // 8–15
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 16–23
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 24–31
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 32–39
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 40–47
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 48–55
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 56–63
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 64–71
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 72–79
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 80–87
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 88–95
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 96–103
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 104–111
                    &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn, &&stn,             // 112–119
                    &&stn, &&stn, &&stn, &&stn, &&loop_whitespace, &&stn, &&stn, &&stn, // 120–127
                };

            const void *jumpTableFound8[256] =
                {
                    &&sChar, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 0–7
                    &&gt1char, &&gt1char, &&sChar, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 8–15
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 16–23
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 24–31
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 32–39
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 40–47
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 48–55
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 56–63
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 64–71
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 72–79
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 80–87
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 88–95
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 96–103
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 104–111
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&gt1char,   // 112–119
                    &&gt1char, &&gt1char, &&gt1char, &&gt1char, &&sChar, &&gt1char, &&gt1char, &&gt1char,   // 120–127
                };

            const void *jumpTableFound16[256] =
                {
                    &&sShort, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char,  // 0–7
                    &&ge2char, &&ge2char, &&sShort, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char,  // 8–15
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 16–23
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 24–31
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 32–39
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 40–47
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 48–55
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 56–63
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 64–71
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 72–79
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 80–87
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 88–95
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 96–103
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 104–111
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&ge2char, // 112–119
                    &&ge2char, &&ge2char, &&ge2char, &&ge2char, &&sShort, &&ge2char, &&ge2char, &&ge2char, // 120–127
                };

            const void *jumpTableFound24[256] =
                {
                    &&ssShort, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, // 0–7
                    &&sIntL, &&sIntL, &&ssShort, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, // 8–15
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 16–23
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 24–31
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 32–39
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 40–47
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 48–55
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 56–63
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 64–71
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 72–79
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 80–87
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 88–95
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 96–103
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 104–111
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,   // 112–119
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&ssShort, &&sIntL, &&sIntL, &&sIntL, // 120–127
                };

            const void *jumpTableIntLoop[256] =
                {
                    &&eol, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,             // 0–7
                    &&sIntL, &&sIntL, &&eol, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,             // 8–15
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 16–23
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 24–31
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 32–39
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 40–47
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 48–55
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 56–63
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 64–71
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 72–79
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 80–87
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 88–95
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 96–103
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 104–111
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&sIntL,           // 112–119
                    &&sIntL, &&sIntL, &&sIntL, &&sIntL, &&loop_whitespace, &&sIntL, &&sIntL, &&sIntL, // 120–127
                };

            goto *jumpTableLoop[*p];
            uint32_t numeral;
            //_|_|_|c-c-c-c ... |

        stn:
            // save current destination
            char *pos = p;
            char roffset = 0;
            goto *jumpTableFound8[*(++p)];  // if the next char after dest is a whitespace, jump to save char, to only save one char in a number
        gt1char:                            // otherwise we will land here
            goto *jumpTableFound16[*(++p)]; // if the 2nd byte after dst is a whitespace, jump to sShort for we got at least 2 letters we will take as 1 int with 16bit offset
        ge2char:                            // otherwise we will land here
            goto *jumpTableFound24[*(++p)]; // if the 3rd byte after dst is a whitespace, jump to ssShort for we got 3 letters we will take as 1 int with 8 bit offset
        sIntL:                              // otherwise we got 4 or more letters in a row, therefor we loop_saveInt
            numeral = *((uint32_t *)pos);   // get numeral at pos and increment pos for next possible step to be in a different position
            pos++;
            obj->numeral_buffer[obj->numNumerals] = numeral; // save numeral to buffer
            obj->numNumerals++;                              // increment number of numerals for correct saving location and later work
            goto *jumpTableIntLoop[*(++p)];                  // if the next byte is also not a whitespace, will land back here, otherwise will loop whitespace

        sChar:
            roffset = 24; // have a 24 offset ready for shift, and jump to save as integer;
            goto sInt;
        sShort: // save 2 byte in an integer by offsetting the current data >> 16
            roffset = 16;
            goto sInt;
        ssShort: // save 3 byte in an int by offsetting the current data >> 8
            roffset = 8;
            goto sInt;
        sInt:
            numeral = *((uint32_t *)pos);
            uint32_t onumeral = numeral >> roffset;                     // get data at destination for exactly the filed bytes we need
            obj->numeral_buffer[obj->numNumerals] = (uint32_t)onumeral; // save the shifted numeral
            obj->numNumerals++;                                         // increase for correct save location
            goto *jumpTableLoop[*p];

        loop_whitespace:
            ++p;
            goto *jumpTableLoop[*p];

        eol:
            return;
        };

        // Zuerst den String aus Index 0 verarbeiten
        numNumerals = 0; // Sicherstellen, dass wir bei 0 anfangen
        processString(reinterpret_cast<char *>(this->descriptor->data[0]), this);
        
        std::unordered_set<uint32_t>* combined_set = new std::unordered_set<uint32_t>(numeral_buffer, numeral_buffer + numNumerals);
        
        return combined_set;
    }
};

#endif