#ifndef PARSER_MNGR_H
#define PARSER_MNGR_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <dlfcn.h>
#include <stdexcept>
#include <functional>
#include <thread>
#include <filesystem>

#include "ThreadWorks.h"
#include "Utillity.h"

using ParserFunc = size_t (*)(const char *line, void *out);

class Parser_mngr
{
public:
    ParserFunc create_parser(const std::string &format);
    //neu
    template <typename T>
    dataSet<T> *parse_multithreaded(const char *buffer, size_t buffer_size, size_t total_lines, const std::string &format, size_t num_threads = std::thread::hardware_concurrency(), size_t start_line = 1) // start_line ist 1 damit wir die Spaltenbeschriftungen überspringen können
    {
        ParserFunc parser = create_parser(format);

        std::function<int(const char *, void *)> parse_line = [parser](const char *line, void *out) { return parser(line, out); };

        printf("creating thread buffer for %zu threads...\n", num_threads);

        // 1. Pro Thread eigenen Buffer + Counter anlegen
        T** thread_buffer = new T*[num_threads];
        size_t* thread_count = new size_t[num_threads];

        // 2. Startzeilen-Offset berechnen
        size_t start_offset = 0;
        size_t skipped = 0;
        while (start_offset < buffer_size && skipped < start_line)
        {
            if (buffer[start_offset] == '\n')
                ++skipped;
            ++start_offset;
        }

        // 3. Threads starten und befüllen lassen
        threaded_line_split<T>(buffer, format.c_str(), buffer_size, num_threads, start_offset, total_lines - start_line, parse_line, thread_buffer, thread_count);

        // 4. Ergebnis zusammenführen
        dataSet<T> *result = new dataSet<T>();
        result->size = 0;
        for (size_t t = 0; t < num_threads; ++t)
            result->size += thread_count[t];

        printf("combining Buffer: Combined Buffer size %zu\n",result->size);

        result->data = (T *)malloc(sizeof(T) * result->size);
        for (size_t t = 0,offset = 0; t < num_threads; ++t)
        {
            memcpy(result->data + offset, thread_buffer[t], sizeof(T) * thread_count[t]);
            offset += thread_count[t];
            delete[] thread_buffer[t];
        }

        return result;
    }

private:
    std::vector<void *> hSoFile;
    std::vector<ParserFunc> parsers;
    int parser_index = 0;

private:
    void *load_func(const std::string &func_name, const std::string &symbol);
    std::string generate_code(const std::string &func_name, const std::string &format);
    void compile_code(const std::string &cpp_code, const std::string &name);
};

#endif 