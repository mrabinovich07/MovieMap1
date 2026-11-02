#pragma once
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <string>
#include <vector>
 //turns title into a franchise key: "Avengers: Age of Ultron" -> "avengers", "The Avengers" -> "avengers"
inline std::string franchiseKey(const std::string& title) {
    std::string s;
    for (unsigned char c : title) s += static_cast<char>(std::tolower(c));

    //keep the part before the colon or dash
    size_t cut = std::min(s.find(':'), s.find(" - "));
    if (cut != std::string::npos) s = s.substr(0, cut);

    //drop leading "the "
    if (s.rfind("the ", 0) == 0) s = s.substr(4);

    //split into words and remove sequel markers
    static const std::set<std::string> markers =  {"ii", "iii", "iv", "v", "vi", "vii", "viii", "ix", "x", "part"};
    std::vector<std::string> words;
    std::istringstream in(s);
    for (std::string w; in >> w;) words.push_back(w);
    while (words.size() > 1) {
        const std::string& last = words.back();
        bool allDigits = !last.empty() && std::all_of(last.begin(), last.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
        if (allDigits || markers.count(last)) words.pop_back();
        else break;
    }

    std::string key;
    for (size_t i = 0; i < words.size(); ++i) {
        if (i) key += " ";
        key += words[i];
    }
    return key;
}