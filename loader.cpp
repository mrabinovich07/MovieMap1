#include "loader.h"
#include "franchise.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

static std::vector<std::string> splitBy(const std::string& s, char delim) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        size_t pos = s.find(delim, start);
        if (pos == std::string::npos) {
            out.push_back(s.substr(start));
            break;
        }
        out.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return out;
}

//IMDb writes missing values as \N
static bool toInt(const std::string& s, int& out) {
    if (s.empty() || s == "\\N") return false;
    try{ out = std::stoi(s);} catch (...) {return false;}
    return true;
}

Dataset loadMovies(const std::string &basicsPath, const std::string &ratingsPath, const std::string& crewPath, int minVotes) {
    Dataset data;
    //Step 1: ratings (tconst -> rating, votes)
    struct Rating {float rating; int votes;};
    std::unordered_map<std::string, Rating> ratings;

    std::ifstream rf(ratingsPath);
    if (!rf) {
        std::cerr <<"Could not open " << ratingsPath << "\n";
        return data;
    }
    std::string line;
    std::getline(rf, line); // skip header
    while (std::getline(rf, line)) {
        auto f = splitBy(line, '\t');
        if (f.size() < 3) continue;
        int votes;
        if (!toInt(f[2], votes)) continue;
        ratings[f[0]] = { std::stof(f[1]), votes };
    }
    // Step 1b: directors (tconst -> sorted director IDs)
    std::unordered_map<std::string, std::vector<int>> directorsOf;
    std::ifstream cf(crewPath);
    if (!cf) {
        std::cerr << "Could not open " << crewPath << "\n";
        return data;
    }
    std::getline(cf, line);  // skip header
    while (std::getline(cf, line)) {
        auto f = splitBy(line, '\t');
        if (f.size() < 2 || f[1] == "\\N") continue;
        std::vector<int> ids;
        for (const auto& nm : splitBy(f[1], ',')) {
            int id;
            if (nm.size() > 2 && toInt(nm.substr(2), id)) ids.push_back(id);  // skip "nm"
        }
        std::sort(ids.begin(), ids.end());
        directorsOf[f[0]] = std::move(ids);
    }
    // Step 2: basics, keeping only movies that pass the filters
    std::ifstream bf(basicsPath);
    if (!bf) {
        std::cerr << "Could not open " << basicsPath << "\n";
        return data;
    }
    std::unordered_map<std::string, int> genreBit;
    std::getline(bf, line);  // skip header
    while (std::getline(bf, line)) {
        auto f = splitBy(line, '\t');
        // tconst, titleType, primaryTitle, originalTitle, isAdult,
        // startYear, endYear, runtimeMinutes, genres
        if (f.size() < 9) continue;
        if (f[1] != "movie") continue;

        Movie m;
        if (!toInt(f[5], m.year) || !toInt(f[7], m.runtime)) continue;

        auto it = ratings.find(f[0]);
        if (it == ratings.end() || it->second.votes < minVotes) continue;

        m.tconst = f[0];
        m.title = f[2];
        m.franchise = franchiseKey(m.title);
        m.rating = it->second.rating;
        m.votes = it->second.votes;
        auto di = directorsOf.find(f[0]);
        if (di != directorsOf.end()) m.directors = di->second;
        if (f[8] != "\\N") {
            for (const auto& g : splitBy(f[8], ',')) {
                auto gi = genreBit.find(g);
                if (gi == genreBit.end()) {
                    if (genreBit.size() >= 32) continue;  // mask holds 32 bits
                    int bit = static_cast<int>(genreBit.size());
                    gi = genreBit.emplace(g, bit).first;
                    data.genreNames.push_back(g);
                }
                m.genreMask |= (1u << gi->second);
            }
        }
        data.movies.push_back(std::move(m));
    }
    return data;
}