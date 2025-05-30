#include "Parser_mngr.h"

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
    std::stringstream code;
    code << "#include <cstring>\n"
         << "#include <cstdint>\n"
         << "#include <cstdlib>\n"
         << "#include <stdio.h>\n"
         << "#include \"constants.h\"\n"
         << "extern \"C\" char* find_and_clean(char* p, char target)\n{\n"
         << "   while (*p && *p != target)\n" 
         << "   {\n" 
         << "       printf(\"%p -- %c : %c \\n\",p,*p,lut[*p]);\n"
         << "       *p = lut[(unsigned char)*p]; ++p;\n"
         << "   }\n"
         << "   return (*p == target) ? p : nullptr;\n"
         << "}\n\n"
         << "extern \"C\" size_t " << func_name << "(char* line, void* out) {\n"
         << "    char* p = line;\n"
         << "    char* end = nullptr;\n"
         << "    uintptr_t* fields = (uintptr_t*)out;\n";

    int arg_index = 0;
    for (size_t i = 0; i < format.size(); ++i) {
        if (format[i] == '%') {
            ++i;
            if (i >= format.size()) break;
            switch (format[i]) {
                case '_':
                    code << "    // Skip field\n";
                    code << "    if (*p == '\"') {\n"
                         << "        p = strchr(p + 1, '\"');\n"
                         << "        if (p) p = strchr(p + 1, ',');\n"
                         << "    } else {\n"
                         << "        p = strchr(p, ',');\n"
                         << "    }\n"
                         << "    if (p) ++p;\n";
                    break;
                case 's':
                    code << "    if (*p == '\"') {\n"
                         << "        ++p;\n"
                         << "        end = find_and_clean(p, '\"');\n"
                         << "        if (end) *end = '\\0';\n"
                         << "        fields[" << arg_index << "] = (uintptr_t)p;\n"
                         << "        p = end + 1;\n"
                         << "        if (*p == ',') ++p;\n"
                         << "    } else {\n"
                         << "        end = find_and_clean(p, ',');\n"
                         << "        if (!end) end = p + strlen(p);\n"
                         << "        fields[" << arg_index << "] = (uintptr_t)p;\n"
                         << "        if (*end != '\\0') *end = '\\0';\n"
                         << "        p = (*end == ',') ? end + 1 : end;\n"
                         << "    }\n";
                    ++arg_index;
                    break;
                case 'd':
                    code << "    fields[" << arg_index << "] = (uintptr_t)atoi(p);\n";
                    code << "    p = strchr(p, ',');\n"
                         << "    if (p) ++p;\n";
                    ++arg_index;
                    break;
                case 'f':
                    code << "    float tmp_f = strtof(p, &end);\n";
                    code << "    uintptr_t tmp_bits;\n"
                         << "    memcpy(&tmp_bits, &tmp_f, sizeof(float));\n"
                         << "    fields[" << arg_index << "] = tmp_bits;\n";
                    code << "    p = (*end == ',') ? end + 1 : end;\n";
                    ++arg_index;
                    break;
                default:
                    break;
            }
        }
    }

    code << "    return p - line;\n}\n";
    return code.str();
}


void Parser_mngr::compile_code(const std::string& cpp_code, const std::string& name) 
{
    std::string filename = name + ".cpp";
    std::string sofile = name + ".so";

    std::ofstream out(filename);
    out << cpp_code;
    out.close();

    std::string cmd = "g++ -std=c++17 -O3 -fPIC -shared -nostdlib -nodefaultlibs " + filename + " -o " + sofile + " -lc";
    if (system(cmd.c_str()) != 0) {
        throw std::runtime_error("Compilerfehler bei: " + filename);
    }
}