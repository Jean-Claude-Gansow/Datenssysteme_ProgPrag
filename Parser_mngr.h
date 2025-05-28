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

#include <filesystem>

using ParserFunc = void(*)(const char* line, void** out);

class Parser_mngr 
{
public:
    ParserFunc create_parser(const std::string& format);
    const std::vector<ParserFunc> &get_all_parsers() const { return parsers; }

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