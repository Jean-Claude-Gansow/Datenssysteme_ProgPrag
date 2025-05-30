#include "DataTypes.h"
#include "Parser_mngr.h"

#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

//
// Created by Jean-Claude on 19.05.2025.
//

#ifndef DUPLICATEDETECTION_FILEINPUT_H
#define DUPLICATEDETECTION_FILEINPUT_H

Parser_mngr* parser_mngr = new Parser_mngr();

char* mmap_file(const char* filename, size_t& filesize) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) return nullptr;

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        close(fd);
        return nullptr;
    }
    filesize = sb.st_size;

    char* data = (char*)mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
    return (data == MAP_FAILED) ? nullptr : data;
}

// Gibt einen Zeiger auf den Anfang der letzten Zeile zurück
inline const char* find_last_line(char* file, size_t file_size) {
    // Starte am letzten Zeichen vor Dateiende
    ssize_t i = file_size - 2; // -1: letztes gültiges Zeichen, -2: vor '\0' oder EOF
    while (i >= 0 && file[i] != '\n') --i;
    return file + i + 1; // +1: Anfang der letzten Zeile
}

void cleanse(char* line) {
    // Beispiel: Alles in Kleinbuchstaben und unerwünschte Zeichen entfernen
    for (char* p = line; *p; ++p) {
        if (*p >= 'A' && *p <= 'Z') *p = *p - 'A' + 'a';
        // Beispiel: Entferne Steuerzeichen außer \n und \0
        if (*p < 32 && *p != '\n' && *p != '\0') *p = ' ';
    }
}


template <typename T>
void* readFile(const char* filename,const char* format)
{
    /* do the data cleaning and storing for later usage here*/
    size_t file_size = 0;
    char* file = mmap_file(filename,file_size);

    auto parse_line = parser_mngr->create_parser("%_,%s");

    size_t line_start = 0;
    size_t line_idx = 0;

    dataSet<T>* dataSet = new struct dataSet<T>(); 
    const char* ll = find_last_line(file, file_size); // find the last line to read its numerical id
    char * end;
    long lines = strtod(ll,&end);
    dataSet->data = (T*)malloc(sizeof(T) * lines); // allocate space for all lines
    printf("attempting data parsing @ %p for %ld lines ...\n",file,lines);   

    for (size_t i = 0; i < file_size-1; ++i) 
    {
        if (file[i] == '\n')  // umarbeiten zu data cleaning switch
        {
            file[i] = '\0'; //substring terminieren
            void* out =  (void*)&dataSet[i]; // Speicher für die Ausgabe reservieren
            
            parse_line(&file[line_start], &out);

            line_start = i + 1;
            ++line_idx;
        }
    }
    
     printf("data parsing successfull...\n");

    return nullptr;
}

#endif //DUPLICATEDETECTION_FILEINPUT_H
