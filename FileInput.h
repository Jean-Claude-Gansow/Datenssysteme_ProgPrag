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
#include <filesystem>

//
// Created by Jean-Claude on 19.05.2025.
//

#ifndef DUPLICATEDETECTION_FILEINPUT_H
#define DUPLICATEDETECTION_FILEINPUT_H

Parser_mngr* parser_mngr = new Parser_mngr();

std::string make_absolute_path(const std::string& rel_path) 
{
    // canonical entfernt auch ".." und "." aus dem Pfad, falls die Datei existiert
    // absolute macht nur einen absoluten Pfad daraus, lässt aber ".." stehen
    try {
        return std::filesystem::canonical(rel_path).string();
    } catch (const std::filesystem::filesystem_error&) {
        // Fallback: absolute, falls Datei noch nicht existiert
        return std::filesystem::absolute(rel_path).string();
    }
}

char* mmap_file(const char* filename, size_t& filesize) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return nullptr;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return nullptr;
    }
    filesize = sb.st_size;
    if (filesize == 0) {
        fprintf(stderr, "Datei ist leer!\n");
        close(fd);
        return nullptr;
    }

    char* data = (char*)mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        data = nullptr;
    }
    close(fd);
    return data;
}

// Gibt einen Zeiger auf den Anfang der letzten Zeile zurück
inline const char* find_last_line(char* file, size_t file_size) {
    // Starte am letzten Zeichen vor Dateiende
    ssize_t i = file_size - 2; // -1: letztes gültiges Zeichen, -2: vor '\0' oder EOF
    while (i >= 0 && file[i] != '\n') --i;
    return file + i + 1; // +1: Anfang der letzten Zeile
}


template <typename T>
void* readFile(const char* filename,const char* format)
{
    /* do the data cleaning and storing for later usage here*/
    size_t file_size = 0;
    std::string absolutePath = make_absolute_path(filename);
    
    printf("attempting to open file: %s\n",absolutePath.c_str());

    char* file = mmap_file(absolutePath.c_str(),file_size);

    auto parse_line = parser_mngr->create_parser(format);

    const char* ll = find_last_line(file, file_size); // find the last line to read its numerical id
    char * end;
    long lines = strtod(ll,&end);

    size_t line_start = 0;
    size_t line_idx = 0;

    dataSet<T>* dataSet = new struct dataSet<T>(); 
    dataSet->data = (T*)malloc(sizeof(T) * lines); // allocate space for all lines
    printf("allocated parser result buffer: %p for %u bytes\n",dataSet->data,sizeof(T)*lines);
    printf("attempting data parsing @ %p for %ld lines ...\n",file,lines);   

    for (size_t i = 0; i < lines; ++i) 
    {
        printf("writing parsing results to: %p\n",&dataSet->data[i]);

        int read = parse_line(&file[line_start], &dataSet->data[i]);

        std::string formatDel = format;
        formatDel+="\n";
        printf(formatDel.c_str(),dataSet->data[i]);

        line_start += read;
        ++line_idx;
    }
    
     printf("data parsing successfull...\n");

    return nullptr;
}

#endif //DUPLICATEDETECTION_FILEINPUT_H
