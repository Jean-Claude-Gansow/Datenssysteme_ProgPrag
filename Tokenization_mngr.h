#ifndef TOKENIZATION_MNGR_H
#define TOKENIZATION_MNGR_H

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
    static const int ALPHABET_SIZE = 36; // Anzahl der Buchstaben im Alphabet
public:
    token_node* child[ALPHABET_SIZE];  // 26 mögliche Buchstaben
    char id_index;    // Dieses Token endet hier
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
        *p = token_lut[*p]; //rework token
        goto *jumpTable[*(++p)]; //goto the scenario, chosen by that character found

        semFound: //found a semicolon, inserting from last token
        *p = 0; //mark end of string
        insert(tokenbegin,lineIndex); //insert, until current position, at current index, no incement, since we stay at this line
        ++p; //move read to next possible character
        tokenbegin = p;
        goto *jumpTable[*(p)]; //jump to whatever we find, should be the first char of the next token

        spaceFound:
        *p = 0;             // mark end of string
        insert(tokenbegin,lineIndex++); // insert, until current position
        ++p;                // move read to next possible character
        tokenbegin = p;
        goto *jumpTable[*(p)]; // jump to whatever we find, should be the first char of the next token

        eof:
        return p-file.data();
    }

    char contains(const std::string &word) const
    {
        const token_node *node = this;
        for (char c : word)
        {
            node = node->child[c - ALPHABET_ANCHOR];
            if (!node)
                return 0;
        }
        return node->id_index; // ist dieser != 0 - Toekn Gefunden und information was es ist gleichzeitig numerisch angegeben
    }

private:
    std::string make_token_compatible(const std::string& token)
    {
        std::string compatible_token = token;
        for (int i = 0; i < token.size();i++)
        {
            compatible_token[i] = token_lut[token[i]];
        }
        return compatible_token;
    }

    void insert(const std::string& word,size_t index)
    {
        token_node* node = this;
        //std::string compatible_word = make_token_compatible(word); //rewrite, for not needed from loading from file, write version with this for direct use from insert 
        for (char c : word)
        {
            int idx = c - ALPHABET_ANCHOR;
            if (!node->child[idx])
                node->child[idx] = new token_node();
            node = node->child[idx];//walk down the tree
        }
        node->id_index = index;//assign valid index
    }

};

template <size_t N>
class Tokenization_mngr
{
public:
    // Token-Liste aus Datei laden (ein Token pro ; pro Zeile)
    bool loadTokenList(const std::string &filename,token_class tk)
    {
        return classes[tk].fimport_token(filename.c_str());        
    }

    void addToken(const std::string &token,token_class  tk)
    {
        classes[tk].insert(token);
    }

    // Prüfen, ob ein Token enthalten ist
    char contains(const std::string &token,token_class tk) const
    {
        for(int i = 0; i < token.size(); i++)
        {
            char index = classes[tk].contains(token);
            if(index != 0){return index;}
        } 
    }

    // Gibt alle Tokens aus text numerisch nach buffer zurück. 
    template<typename t> 
    void filterTokens(char *text,t* buffer) const
    {
        void *jumpTable[128] = {
            &&eof, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,        // 0–7
            &&loop, &&loop, &&loop_whitespace, &&loop, &&loop, &&loop, &&loop, &&loop, // 8–15
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 16–23
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 24–31
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 32–39
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 40–47
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 48–55
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,     // 56–63
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 64–71
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 72–79
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 80–87
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 88–95
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 96–103
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 104–111
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop,       // 112–119
            &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop, &&loop        // 120–127
        };

        void *jumpTableWhite[128] = {
            &&eof, &&check, &&check, &&check, &&check, &&check, &&check, &&check,             // 0–7
            &&check, &&check, &&check_whitesapce, &&check, &&check, &&check, &&check, &&check, // 8–15
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 16–23
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 24–31
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 32–39
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 40–47
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 48–55
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 56–63
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 64–71
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 72–79
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 80–87
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 88–95
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 96–103
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 104–111
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check,            // 112–119
            &&check, &&check, &&check, &&check, &&check, &&check, &&check, &&check             // 120–127
        };

        bool check[N];
        memset(check,true,N); 
        char* p = text;

        goto check;

        loop: //no matter what is encountered exept for whitespace we skip
        goto *jumpTable[*(p++)];
        
        loop_whitepace: //no matter what other than whitespace is encountered, we check for tokens
        goto *jumpTableWhite[*(p++)];
        
        check: //we encountered something after whitespace, that isnt whitespace, we check for a token
        for(int i = 0; i < N;++i)
        {
            char index = contains(std::string(p),i); //check whether the word at the current pointer is a token of class i
            if(index > 0)
            {
                (char*)buffer[i] = index;
                goto *jumpTable[*(p++)]; //if a token was identified, we continue looking for further tokens in coming words.
            }
        }
        goto *jumpTable[*(p++)];
        
        eof:
        return;
    }

private:
    int numClasses = N; 
    token_node classes[N]; 


    // Hilfsfunktion zum Trimmen von Strings
    static void trim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)          { return !std::isspace(ch); }).base(),s.end());
    }
};

#endif // TOKENIZATION_MNGR_H