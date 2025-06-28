#include "Parser_mngr.h"
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

using ParserFunc = size_t(*)(const char* line, void* out);

ParserFunc Parser_mngr::create_parser(const std::string& format) {
    // 1. Code generieren (Beispiel, passe an dein Format an)

    std::string name = "parser_" + std::to_string(parsers.size());
    
    std::string code = generate_code(name,format);

    // 2. Kompilieren
    compile_code(code, name);

    // 3. Laden
    void* fn_ptr = load_func(name, name); // "parse_line" ist der Name der generierten Funktion
    
    // 4. Cast und speichern
    ParserFunc fn = reinterpret_cast<ParserFunc>(fn_ptr);
    parsers.push_back(fn);

    return fn;
}

void* Parser_mngr::load_func(const std::string& func_name, const std::string& symbol) 
{
    printf("loading function...\n");
    std::string sofile = func_name + ".so";
    std::string abs_path = std::filesystem::absolute(sofile).string();
    
    printf("attempting to open file: %s\n",abs_path.c_str());

    void* handle = dlopen(abs_path.c_str(), RTLD_NOW);

    if (!handle) {
        throw std::runtime_error("Fehler beim Laden von " + sofile + ": " + dlerror());
    }

    dlerror(); // Clear any existing error
    void* sym = dlsym(handle, symbol.c_str());
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        dlclose(handle);
        throw std::runtime_error("Fehler beim Laden des Symbols " + symbol + ": " + dlsym_error);
    }

   

    hSoFile.push_back(handle); // Optional: handle verwalten für späteres dlclose()

    printf("done loading...\n");

    return sym;
}

std::string Parser_mngr::generate_code(const std::string& func_name, const std::string& format) {
    std::string template_code = read_file("parser_template.cpp");

    std::stringstream format_code;
    int arg_index = 0;
    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] == '%') {
            ++i;
            if (i >= format.size()) break;
            switch (format[i]) {
                case '_':
                    format_code << "    parse_field_ignore(p, line, outbuffer);\n";
                    break;
                case 's':
                    format_code << "    parse_field_s(p, fields, " << arg_index << ", line, outbuffer);\n";
                    ++arg_index;
                    break;
                case 'f':
                    format_code << "    parse_field_f(p, fields, " << arg_index << ", line, outbuffer);\n";
                    ++arg_index;
                    break;
                case 'V':
                    format_code << "    parse_field_V(p, fields, " << arg_index << ", line, outbuffer);\n";
                    ++arg_index;
                    break;
                case 'd':
                    format_code << "    parse_field_d(p, fields, " << arg_index << ", line, outbuffer);\n";
                    ++arg_index;
                    break;
                default:
                    format_code << "    // Unbekannter Feldtyp: " << format[i] << "\n";
                    break;
            }
        }
    }

    // Platzhalter ersetzen wie gehabt
    size_t pos;
    while ((pos = template_code.find("{{FUNC_NAME}}")) != std::string::npos)
        template_code.replace(pos, 13, func_name);
    while ((pos = template_code.find("{{FORMAT_CODE}}")) != std::string::npos)
        template_code.replace(pos, 15, format_code.str());

    return template_code;
}


void Parser_mngr::compile_code(const std::string& cpp_code, const std::string& name) 
{
    std::string filename = name + ".cpp";
    std::string sofile = name + ".so";

    std::ofstream out(filename);
    out << cpp_code;
    out.close();

    std::string cmd = "g++ -std=c++17 -g -O3 -fPIC -shared -nostdlib -nodefaultlibs " + filename + " -o " + sofile + " -lc";
    if (system(cmd.c_str()) != 0) {
        throw std::runtime_error("Compilerfehler bei: " + filename);
    }
}

