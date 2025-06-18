#ifndef TOKENIZATION_MNGR_H
#define TOKENIZATION_MNGR_H

#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <algorithm>

class TokenizationMngr
{
public:
    // Token-Liste aus Datei laden (ein Token pro Zeile)
    bool loadTokenList(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return false;
        std::string line;
        while (std::getline(file, line))
        {
            trim(line);
            if (!line.empty())
                tokens.insert(line);
        }
        return true;
    }

    // Einzelnes Token hinzufügen
    void addToken(const std::string &token)
    {
        tokens.insert(token);
    }

    // Prüfen, ob ein Token enthalten ist
    bool contains(const std::string &token) const
    {
        return tokens.find(token) != tokens.end();
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
    std::unordered_set<std::string> tokens;

    // Hilfsfunktion zum Trimmen von Strings
    static void trim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)          { return !std::isspace(ch); }).base(),s.end());
    }
};

#endif // TOKENIZATION_MNGR_H