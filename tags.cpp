//
// Created by mitch on 11/1/2026.
//
#include "tags.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <unordered_map>

//splits one CSV line, handling "quoted, fields" and "" escapes
static std::vector<std::string> parseCsvLine(std::string line) {
if (!line.empty() && line.back() == '\r') line.pop_back();
    std::vector<std::string> out;
    std::string cur;
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i+1] == '"') {cur += '"'; ++i; }
                else inQuotes = false;
            } else cur += c;
        } else if (c == '"') inQuotes = true;
        else if (c == ',') {out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}

static std::string cleanTag(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    size_t a = s.find_first_not_of(' ');
    size_t b = s.find_last_not_of(' ');
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}

size_t attachTags(Dataset &data, const std::string &linksPath, const std::string &tagsPath, size_t minMovies) {
    std::unordered_map<int, int> imdbToIndex;
    for (size_t i = 0; i < data.movies.size(); ++i) {
        const std::string& t = data.movies[i].tconst;
        if (t.size() > 2) {
            try { imdbToIndex[std::stoi(t.substr(2))] = static_cast<int>(i); }
            catch (...){}
        }
    }

    std::ifstream lf(linksPath);
    if (!lf) { std::cerr << "Could not open " << linksPath << "\n"; return 0; }
    std::unordered_map<int, int> mlToIndex;
    std::string line;
    std::getline(lf, line);
    while (std::getline(lf, line)) {
        auto f = parseCsvLine(line);
        if (f.size() < 2) continue;
        try {
            auto it = imdbToIndex.find(std::stoi(f[1]));
            if (it != imdbToIndex.end()) mlToIndex[std::stoi(f[0])] = it->second;
        } catch (...){}
    }

    std::ifstream tf(tagsPath);
    if (!tf) { std::cerr << "Could not open " << tagsPath << "\n"; return 0; }
    std::unordered_map<std::string, std::vector<int>> moviesWithTag;
    std::getline(tf, line);
    while (std::getline(tf, line)) {
        auto f = parseCsvLine(line);
        if (f.size() < 3) continue;
        std::string tag = cleanTag(f[2]);
        if (tag.empty()) continue;
        int ml;
        try { ml = std::stoi(f[1]); } catch (...) {continue; }
        auto mi = mlToIndex.find(ml);
        if (mi == mlToIndex.end()) continue;
        moviesWithTag[tag].push_back(mi->second);
    }

    for (auto& entry : moviesWithTag) {
        std::vector<int>& list = entry.second;
        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());
        if (list.size() < minMovies) continue;
        const int id = static_cast<int>(data.tagNames.size());
        data.tagNames.push_back(entry.first);
        for (int idx : list) data.movies[idx].tags.push_back(id);
    }

    size_t tagged = 0;
    for (Movie& m : data.movies) {
        std::sort(m.tags.begin(), m.tags.end());
        if (!m.tags.empty()) ++tagged;
    }
    return tagged;
}
