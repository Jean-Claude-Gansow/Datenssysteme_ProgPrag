#ifndef TOKENIZATION_MNGR_H
#define TOKENIZATION_MNGR_H

#include <string>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>

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
class token_node {
public:
    token_node* child[26];  // 26 mögliche Buchstaben
    bool is_terminal;          // Dieses Token endet hier

    token_node() : is_terminal(false) {
        for (int i = 0; i < 26; ++i)
            child[i] = nullptr;
    }

    ~token_node() {
        for (int i = 0; i < 26; ++i)
            delete child[i];
    }

    void insert(const std::string& word) {
        token_node* node = this;
        for (char c : word) {
            int idx = c - 'a';
            if (!node->child[idx])
                node->child[idx] = new token_node();
            node = node->child[idx];
        }
        node->is_terminal = true;
    }

    bool contains(const std::string& word) const {
        const token_node* node = this;
        for (char c : word) {
            node = node->child[c - 'a'];
            if (!node) return false;
        }
        return node->is_terminal;
    }
};



class TokenizationMngr
{
public:

    
    // Token-Liste aus Datei laden (ein Token pro Zeile)
    bool loadTokenList(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {return false;}

        std::string line;
        while (std::getline(file, line))
        {
            trim(line);
            if (!line.empty())
                data.insert(line);
        }
        return true;
    }

    // Einzelnes Token hinzufügen
    void addToken(const std::string &token)
    {
        data[token[0] - 'a'].insert(token);
    }

    // Prüfen, ob ein Token enthalten ist
    bool contains(const std::string &token) const
    {
        return data.find(token) != tokens.end();
    }

    // Tokenisiere einen String anhand von Whitespaces
    static std::vector<std::string> tokenize(const std::string &text)
    {
        std::vector<std::string> result;
        std::istringstream iss(text);
        std::string token;
        while (iss >> token)
        {
            result.push_back(token);
        }
        return result;
    }

    // Gibt alle Tokens aus text zurück, die in der Token-Liste enthalten sind
    std::vector<std::string> filterTokens(const std::string &text) const
    {
        std::vector<std::string> result;
        for (const auto &token : tokenize(text))
        {
            if (contains(token))
                result.push_back(token);
        }
        return result;
    }

private:
    token_node data[26]; // 26 Token-Bäume für jeden Buchstaben


    // Hilfsfunktion zum Trimmen von Strings
    static void trim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)          { return !std::isspace(ch); }).base(),s.end());
    }
};

#endif // TOKENIZATION_MNGR_H