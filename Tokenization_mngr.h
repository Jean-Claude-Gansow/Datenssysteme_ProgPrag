#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "FileInput.h"
#include "DataTypes.h"
#include "Utillity.h"


#ifndef TOKENIZATION_MNGR_H
#define TOKENIZATION_MNGR_H



//Aufbau: Tokenization Manager bekommt für jeden Buchstaben einen Token tree, dieser enthält absteigend die Tokens, die mit diesem Buchstaben anfangen. Alsbald ein buchstabe abweicht, wird in der Liste nachgesehen ob es ein Wort gibt, welches den aktuellen buchstaben an der aktuellen Position enthält

//Beispiel:
// accessoires
// accueil
// adata

//scenario: Wir finden einen token mit a und betreten den Baum bei accessoires
//scenario: der zweite buchstabe ist ein c, also bleiben wir im tokentree bei accessoires
//scenario: der dritte buchstabe ist ein c, also bleiben wir im tokentree
//scenario: der vierte Buchstabe ist ein u, springe in der Liste zum index u - 'a' und springe somit in den Baum von accueil
//scenario - Fall 1: das Wort ist accueil, also ist es ein Token
//scenario - Fall 2: das Wort ist ein nicht im tokenizer bekanntes wort, also wird es nicht als Token erkannt

//rework, try to use a tree structure for the tokens, so that we can use a lookup table to find the next letter and then search for the next token in the children of the current token
class token_node
{
public:
    static const char ALPHABET_ANCHOR = 'W'; 
    static const int ALPHABET_SIZE = 36+3; // Anzahl der Buchstaben im Alphabet + 2 hilfszeichen für leerschritte 
public:
    token_node* child[ALPHABET_SIZE];  // 36 mögliche Buchstaben
    size_t id_index;    // Dieses Token endet hier
    token_node() : id_index(0)
    {
        memset(this->child,0,sizeof(this->child));
    }
    ~token_node()
    {
        for (int i = 0; i < ALPHABET_SIZE; ++i)
            delete child[i];
    }

    size_t fimport_token(const char *filename)
    {
        void* jumpTable[128] = {
            &&eof, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 0–7
            &&loop, &&loop, &&spaceFound, &&loop, &&loop, &&loop, &&loop, &&loop,     // 8–15
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 16–23
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 24–31
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 32–39
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 40–47
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 48–55
            &&loop, &&loop, &&loop, &&semFound, &&loop, &&loop, &&loop, &&loop, // 56–63
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 64–71
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 72–79
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 80–87
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 88–95
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 96–103
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 104–111
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 112–119
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop      // 120–127
        };

        File file(filename,true);

        char* p = file.data();
        char* tokenbegin = p;
        size_t lineIndex = 1; //start at 1 for our id_index in the nodes is 0 if not a final node
        
        loop: //rewrite numeric characters to match map indentation
        *p = lut[*p]; //rework token
        goto *jumpTable[*(++p)]; //goto the scenario, chosen by that character found

        semFound: //found a semicolon, inserting from last token
        *p = 0; //mark end of string
        insert(tokenbegin,lineIndex,(p-tokenbegin)); //insert, until current position, at current index, no incement, since we stay at this line
        ++p; //move read to next possible character
        tokenbegin = p;
        goto *jumpTable[*(p)]; //jump to whatever we find, should be the first char of the next token

        spaceFound:
        *p = 0;             // mark end of string
        insert(tokenbegin, lineIndex++,(p - tokenbegin)-1); // insert, until current position
        ++p;                // move read to next possible character
        tokenbegin = p;
        goto *jumpTable[*(p)]; // jump to whatever we find, should be the first char of the next token

        eof:
        *p = 0;                                          // mark end of string
        insert(tokenbegin, lineIndex++, (p - tokenbegin)); // insert, until current position
        return lineIndex; // return the number of tokens with different meaning
    }

    token get_possible_index(char *p) const
    {
        void* jumpTable[ALPHABET_SIZE] = {};

        const token_node *node = this;
        char *current = p;

        while (*current)
        {
            unsigned char c = *current;
            //if (c < ALPHABET_ANCHOR) //sollte nicht passieren können
                //return 0; // Optional: Zeichen außerhalb des gültigen Bereichs
            if(c < ALPHABET_ANCHOR || c > 127){*current = whitespace;c = *current;}
            //printf("jumping to childnode: %c\n",c);
            node = node->child[c - ALPHABET_ANCHOR];
            
            if (node == nullptr)
            {
                return 0;
            }
            else if (node->id_index > 0)
            {
                return node->id_index;
            }
            ++current;
        }

        return 0;
    }

    void print() const {
        print_trie(this, "", '\0', true);
    }

    void print_trie(const token_node *node, const std::string &prefix, char edge, bool isLast) const
    {
        if (!node)
            return;

        // Nur anzeigen, wenn der aktuelle Buchstabe gesetzt ist (außer Wurzel)
        if (edge != '\0')
        {
            std::cout << prefix;
            std::cout << (isLast ? " └─ " : " ├─ ");
            std::cout << edge;

            if (node->id_index != 0)
            {
                std::cout << " : index(" << static_cast<int>(node->id_index) << ")";
            }
            std::cout << "\n";
        }

        // Neue Prefix für die nächste Ebene
        std::string newPrefix = prefix + (isLast ? "    " : " │  ");

        // Zähle echte Kinder (nicht-null)
        std::vector<int> nonNullIndices;
        for (int i = 0; i < 39; ++i)
        {
            if (node->child[i])
            {
                nonNullIndices.push_back(i);
            }
        }

        // Für jeden Kindknoten rekursiv aufrufen
        for (size_t i = 0; i < nonNullIndices.size(); ++i)
        {
            int idx = nonNullIndices[i];
            char nextChar = static_cast<char>(idx + ALPHABET_ANCHOR);
            bool last = (i == nonNullIndices.size() - 1);
            print_trie(node->child[idx], newPrefix, nextChar, last);
        }
    }

private:
    void insert(const char* word,size_t index,size_t len)
    {
        token_node* node = this;
        //std::string compatible_word = make_token_compatible(word); //rewrite, for not needed from loading from file, write version with this for direct use from insert 
        for (int i = 0; i < len; i++)
        {
            int idx = word[i] - ALPHABET_ANCHOR;
            if (node->child[idx] == nullptr)
            {
                node->child[idx] = new token_node();
            }
            node = node->child[idx];//walk down the tree
        }
        node->id_index = index;//assign valid index
    }

};


inline static size_t numTokenizers = 0; //keep track of how many tokenizers there is

template <size_t N,typename in_buf_t,typename out_buf_t>
class Tokenization_mngr
{
    using TokenizerFunc = size_t (*)(in_buf_t *bufferEntry, token**out, Tokenization_mngr *tkm);
public:
    size_t m_class_tokens_found[N];
public:
    Tokenization_mngr(std::vector<std::string> template_types)
    {
        this->m_tokenizer_mngr_id = numTokenizers++;
        for(std::string s : template_types)
        {
            this->template_type_str.push_back(s);
        }
    }

    Tokenization_mngr(std::vector<std::string> template_types,const category priority[N])
    {
        this->m_tokenizer_mngr_id = numTokenizers++;
        for (std::string s : template_types)
        {
            this->template_type_str.push_back(s);
        }
    }

    // Token-Liste aus Datei laden (ein Token pro ; pro Zeile)
    bool loadTokenList(const std::string &filename, token_class tk)
    {
        printf("importing file: %s\n", filename.c_str());
        size_t ret = classes[tk].fimport_token(filename.c_str());
        this->m_class_tokens_found[tk] += ret;
        classes[tk].print();
        return true;     
    }

    void addToken(const std::string &token,token_class tk)
    {
        classes[tk].insert(token);
    }

    // Prüfen, ob ein Token enthalten ist
    token contains(char* token,token_class tk) const
    {
        return classes[tk].get_possible_index(token);
    }

    dataSet<out_buf_t>* tokenize_multithreaded(dataSet<in_buf_t>* ds,const char* format,size_t num_threads)
    {
        TokenizerFunc tokenizer = this->create_tokenizer(format);

        std::function<int(in_buf_t*, token**, Tokenization_mngr*)> tokenize_field = [tokenizer](in_buf_t* line, token**out, Tokenization_mngr* tkm) { return tokenizer(line, out, tkm); };

        printf("creating thread buffer for %zu threads...\n", num_threads);

        // 1. Pro Thread eigenen Buffer + Counter anlegen
        out_buf_t** thread_buffer = new out_buf_t*[ds->size];
        
        //TODO für morgen: thread counts o.ä. mit einbauen um das zusammenfügen leichter zu machen
        threaded_tokenization(ds->data,ds->size,tokenize_field,num_threads,thread_buffer);
        
        dataSet<out_buf_t>* ret = new dataSet<out_buf_t>();
        ret->data = new out_buf_t[ds->size];

        //füge Buffer zusammen
        for(int ind = 0,offset = ds->size/num_threads; ind < num_threads;ind++)
        {
            printf("Attaching Buffer %d of size %d to dataSet[%d]\n",ind,offset,offset*ind);
            memcpy(&ret->data[offset*ind],thread_buffer[ind],offset);
        }
        return ret;
    }

    TokenizerFunc create_tokenizer(const char* format)
    {
        std::string name = "tokenizer_" + std::to_string(this->m_tokenizer_mngr_id) + "_" + std::to_string(tokenizers.size());

        std::string code = generate_code(name,format);

        // 2. Kompilieren
        compile_code(code, name);

        // 3. Laden
        void* fn_ptr = load_func(name, name); // "tokenize_line" ist der Name der generierten Funktion
        
        // 4. Cast und speichern
        TokenizerFunc fn = reinterpret_cast<TokenizerFunc>(fn_ptr);
        tokenizers.push_back(fn);
        return fn;
    }

    void filter_tokens(char *text, out_buf_t **buffer)
    {
        char *p = text;

        while (*p)
        {
            // Überspringe Whitespaces
            while (*p && *p==whitespace)
            {
                ++p;
            }

            if (!*p)
                break; // EOL erreicht

            // Token-Suche starten
            bool matched = false;
            for (int i = 0; i < N; ++i)
            {
                token index = contains(p, static_cast<category>(i));
                if (index > 0)
                {
                    m_class_tokens_found[i]++;
                    // Speicher den Index des gefundenen Tokens
                    (*((token**)buffer))[i] = index;

                    // p um die Länge des gefundenen Tokens weiterschieben
                    while (*p && *p!=whitespace)
                        ++p;
                    matched = true;
                    break;
                }
            }

            if (!matched)
            {
                // Kein Token erkannt → weiter zum nächsten Wort
                while (*p && *p != whitespace)
                    ++p;
            }
        }
        /*
        printf("looking for toklens in: %s\n",text);
        
        printf("filtering: %s\n",text);
        void *jumpTable[256] = {
            &&eol, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,             // 0–7
            &&loop, &&loop, &&eol, &&loop, &&loop, &&loop, &&loop, &&loop,             // 8–15
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 16–23
            &&loop_whitespace, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, // 24–31
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 32–39
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 40–47
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 48–55
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 56–63
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 64–71
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 72–79
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 80–87
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 88–95
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 96–103
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 104–111
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 112–119
            &&loop, &&loop, &&loop, &&loop, &&loop_whitespace, &&loop, &&loop, &&loop, // 120–127
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 0–7
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 8–15
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 16–23
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 24–31
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 32–39
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 40–47
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 48–55
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 56–63
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 64–71
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 72–79
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 80–87
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 88–95
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 96–103
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 104–111
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,            // 112–119
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop             // 120–127
        };

        void *jumpTableWhite[256] = {
            &&eol, &&check, &&check, &&check, &&check, &&check, &&check, &&check,             // 0–7
            &&check, &&check, &&eol, &&check, &&check, &&check, &&check, &&check,             // 8–15
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 16–23
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 24–31
            &&loop_whitespace, &&check, &&check, &&check, &&check, &&check, &&check, &&check, // 32–39
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 40–47
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 48–55
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 56–63
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 64–71
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 72–79
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 80–87
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 88–95
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 96–103
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 104–111
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 112–119
            &&check, &&check, &&check, &&check, &&loop_whitespace, &&check, &&check, &&check,        // 120–127
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 0–7
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 8–15
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 16–23
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 24–31
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 32–39
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 40–47
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 48–55
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 56–63
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 64–71
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 72–79
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 80–87
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 88–95
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 96–103
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 104–111
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,           // 112–119
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check            // 120–127
        };

        char *p = text;

        goto check;

        loop: // no matter what is encountered exept for whitespace we skip
        printf("-");
        goto *jumpTable[*((unsigned char*)(p++))];

        loop_whitespace: // no matter what other than whitespace is encountered, we check for tokens
        printf("_");
        goto *jumpTableWhite[*((unsigned char*)(p++))];

        check: // we encountered something after whitespace, that isnt whitespace, we check for a token
        printf("|");
        for (int i = 0; i < N;++i)
        {   
            printf("checking category:%d\n",i);
            char index = contains(p, static_cast<category>(i)); // check whether the word at the current pointer is a token of class i
            printf("\n");
            if (index > 0)
            {
                printf("found Token %d!\n", (int)index);
                *((char **)buffer)[i] = index;
                goto *jumpTable[*((unsigned char*)(p))]; // if a token was identified, we continue looking for further tokens in coming words.
            }
        }
        goto *jumpTable[*((unsigned char*)(p++))];

        eol:
        printf("\n");
        return;*/
    }

private:
    inline void threaded_tokenization(in_buf_t *buffer, size_t buffer_size, std::function<int(in_buf_t*, token**,Tokenization_mngr* tkm)> tokenize_entry, size_t num_threads,out_buf_t** thread_buffers)
    {
        size_t *offsets = new size_t[num_threads + 1];
        
        // 1. Bereiche grob aufteilen
        size_t buffered_lines = buffer_size / num_threads;
        offsets[num_threads] = buffer_size; // dont allow reads over the end of the file
        for (size_t t = 0; t < num_threads; ++t)
        {
            offsets[t] = t * buffered_lines;
            thread_buffers[t] = new out_buf_t[buffered_lines]; // allocated buffer per thread
            
        }

        // 4. Threads starten
        std::thread* threads = new std::thread[num_threads];
        for (size_t t = 0; t < num_threads; ++t)
        {
            printf("Thread %zu: -> [%zu - %zu]\n", t, offsets[t], offsets[t + 1]);
            size_t block_start = offsets[t];
            size_t block_end = offsets[t + 1];
            out_buf_t* buffer_ptr = thread_buffers[t];
            size_t buffer_size = block_end - block_start;

            printf("starting thread %zu\n",t);
            threads[t] = std::thread([=, &tokenize_entry]()
            {
                size_t bufferline = block_start;
                size_t out_idx = 0;

                for (int i = block_start; i < block_end; i++)
                {
                    //printf("buffer[%d]: %s",i,(token*)(buffer[i].data[0]));
                    tokenize_entry(&buffer[i],(token**)(&buffer_ptr),this);
                }
            });
        }

        // 5. Warten auf alle Threads
        for (size_t t = 0; t < num_threads; ++t)
            threads[t].join();

        delete[] offsets;
        delete[] threads;
    }

    void *load_func(const std::string &func_name, const std::string &symbol)
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

    // Hilfsfunktion zum Lesen einer Datei als String


    std::string generate_code(const std::string &func_name, const std::string &format)
    {
        std::string template_code = read_file("tokenizer_template.cpp");

        std::stringstream format_code;
        int arg_index = 0;
        for (size_t i = 0; i < format.size(); ++i) 
        {
            if (format[i] == '%') 
            {
                ++i;
                if (i >= format.size()) break;
                switch (format[i]) 
                {
                    case '_':
                        break;
                    case 's':
                        format_code << "\ttkm->filter_tokens((char*)(line->data["<<arg_index<<"]), out);\n";
                        ++arg_index;
                        break;
                    case 'f':
                        ++arg_index;
                        break;
                    case 'V':
                        format_code << "\ttkm->filter_tokens((char*)(line->data[" << arg_index << "]), out);\n";
                        ++arg_index;
                        break;
                    case 'd':
                        ++arg_index;
                        break;
                    default:
                        format_code << "    // Unbekannter Feldtyp: " << format[i] << "\n";
                        break;
                }
            }    
        }

        std::stringstream template_types;
        template_types  << this->template_type_str[0] <<"," << this->template_type_str[1] << "," << this->template_type_str[2];
        std::stringstream temp_str;
        temp_str << "size_t N, typename " << this->template_type_str[1] << ", typename " << this->template_type_str[2];

        // Platzhalter ersetzen wie gehabt
        replace_all(template_code, "{{FUNC_NAME}}", func_name);
        replace_all(template_code, "{{TEMP_TYPES}}", template_types.str());
        replace_all(template_code, "{{TEMP_TYPE_IN}}", this->template_type_str[1]);
        replace_all(template_code, "{{TEMP_TYPE_OUT}}", this->template_type_str[2]);
        replace_all(template_code, "{{FORMAT_CODE}}", format_code.str());

        return template_code;
    }
    
    void compile_code(const std::string &cpp_code, const std::string &name)
    {
        std::string filename = name + ".cpp";
        std::string sofile = name + ".so";

        printf("creating tokenizer %s\n",filename.c_str());

        std::ofstream out(filename);
        out << cpp_code;
        out.close();

        std::string cmd = "g++ -std=c++17 -g -O3 -fPIC -shared -nostdlib -nodefaultlibs " + filename + " -o " + sofile + " -lc";
        if (system(cmd.c_str()) != 0)
        {
            throw std::runtime_error("Compilerfehler bei: " + filename);
        }
    }

    int numClasses = N; 
    token_node classes[N]; 
    size_t  m_tokenizer_mngr_id = 0;
    std::vector<void *> hSoFile;
    std::vector<TokenizerFunc> tokenizers;
    std::vector<std::string> template_type_str;
};

#endif // TOKENIZATION_MNGR_H