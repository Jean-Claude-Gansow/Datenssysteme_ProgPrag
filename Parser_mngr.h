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

using ParserFunc = size_t(*)(const char* line, void* out);

class Parser_mngr 
{
public:
    ParserFunc create_parser(const std::string& format);
    const std::vector<ParserFunc> &get_all_parsers() const { return parsers; }

    // NEU: Multi-Threaded Parsing
    template<typename T>
    dataSet<T>* parse_multithreaded(const char* buffer, size_t buffer_size, size_t total_lines,const std::string& format, size_t num_threads = std::thread::hardware_concurrency())
    {
        ParserFunc parser = create_parser(format);

        // Wrapper für std::function
        std::function<int(const char*, void*)> parse_line = [parser](const char* line, void* out) {return parser(line, out);};

        // Speicher für alle Zeilen anlegen
        dataSet<T>* result = new dataSet<T>();
        result->data = (T*)malloc(sizeof(T) * total_lines);
        result->size = total_lines;
        
        threaded_line_split<T>(buffer, buffer_size, 0, buffer_size, num_threads, total_lines, parse_line, static_cast<T*>(result->data), format.c_str());
        return result;
    }

private:
    std::vector<void*> hSoFile;
    std::vector<ParserFunc> parsers;
    int parser_index = 0;

private:
    void* load_func(const std::string& func_name, const std::string& symbol);
    std::string generate_code(const std::string& func_name, const std::string& format);
    void compile_code(const std::string& cpp_code, const std::string& name);
};


#endif