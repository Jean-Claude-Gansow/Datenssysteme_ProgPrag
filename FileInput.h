#ifndef DUPLICATEDETECTION_FILEINPUT_H
#define DUPLICATEDETECTION_FILEINPUT_H

#include "DataTypes.h"
#include "Parser_mngr.h"
#include "Utillity.h"

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
#include <stdexcept>

class File {
public:
    File(const std::string& path,bool countLines = false) : filename(make_absolute_path(path)), buffer(nullptr), filesize(0)
    {
        buffer = mmap_file(filename.c_str(), filesize);
        if (!buffer) throw std::runtime_error("Konnte Datei nicht mappen: " + filename);
        if(countLines)
            lines = count_lines(); // Zeilen zählen, wenn lineMethod aktiviert ist
        else
            // Nur die letzte Zeile lesen, wenn lineMethod deaktiviert ist
            lines = read_number_from_last_line();
        printf("Datei %s gemappt, Größe: %zu Bytes, Zeilenanzahl: %zu\n", filename.c_str(), filesize, lines);
    }

    ~File() {
        if (buffer && filesize > 0)
            munmap((void*)buffer, filesize);
    }

    size_t line_count() const { return this->lines; }
    char* data() const { return buffer; }
    size_t size() const { return filesize; }
    const std::string& path() const { return filename; }

    // Liest die Zahl am Anfang der letzten Zeile aus
    int read_number_from_last_line() const 
    {
        const char* last_line = find_last_line();
        char* endptr = nullptr;
        long num = std::strtol(last_line, &endptr, 10);
        if (last_line == endptr) {
            throw std::runtime_error("Keine Zahl am Anfang der letzten Zeile gefunden in: " + filename);
        }
        return static_cast<int>(num+2); // +1 weil die ids bei 0 beginnen und wir eine beschriftungszeile haben
    }

    const size_t count_lines() const 
    {
        size_t count = 0;
        for (size_t i = 0; i < filesize; ++i) 
        {
            if (buffer[i] == '\n') ++count;
        }
        return ++count; //\0 mit einrechnen
    }

    // Gibt einen Zeiger auf den Anfang der letzten Zeile zurück
    const char* find_last_line() const 
    {
        ssize_t i = filesize - 2;
        while (i >= 0 && buffer[i] != '\n') --i;
        return buffer + i + 1;
    }

private:
    std::string filename;
    char* buffer;
    size_t filesize;
    size_t lines;

    std::string make_absolute_path(const std::string& rel_path) {
        try {
            return std::filesystem::canonical(rel_path).string();
        } catch (const std::filesystem::filesystem_error&) {
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
};



#endif //DUPLICATEDETECTION_FILEINPUT_H
