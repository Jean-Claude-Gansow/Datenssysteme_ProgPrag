#include "Tokenization_mngr.h"

void* Parser_mngr::load_func(const std::string& func_name, const std::string& symbol) 
{
   
}

// Hilfsfunktion zum Lesen einer Datei als String
std::string read_file_as_string(const std::string& filename) {
    std::ifstream in(filename);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

std::string Parser_mngr::generate_code(const std::string& func_name, const std::string& format) {
    std::string template_code = read_file_as_string("parser_template.cpp");

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

