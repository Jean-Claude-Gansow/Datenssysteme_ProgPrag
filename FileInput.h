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

void munmap_file(char* addr, size_t filesize) {
    munmap(addr, filesize);
}

void* readFile(const char* filename,const char* format)
{
    /* do the data cleaning and storing for later usage here*/
    size_t file_size = 0;
    char* file = mmap_file(filename,file_size);

    auto parser = parser_mngr->create_parser("%_,%s");

    size_t line_start = 0;
    size_t line_idx = 0;
    
    printf("attempting data parsing on %d bytes...\n");

    for (size_t i = 0; i < file_size-1; ++i) 
    {
        if (file[i] == '\n')  // arbeite um zu data cleaning switch
        {
            file[i] = '\0'; // Zeile terminieren
            void* out[10];  // Passe Anzahl an Format an
            parser(&file[line_start], out);

            // Beispiel: Zugriff auf Werte
            // char* name = (char*)out[0];
            // float wert = *(float*)out[1];

            line_start = i + 1;
            ++line_idx;
        }
    }
    
     printf("data parsing successfull...\n");

    return nullptr;
}

#endif //DUPLICATEDETECTION_FILEINPUT_H
