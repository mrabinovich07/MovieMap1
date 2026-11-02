#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct Movie {
    std::string tconst; //IMDb ID
    std::string title;
    int year = 0;
    int runtime = 0;
    int votes = 0;
    float rating = 0.0f;
    uint32_t genreMask = 0; //Only one bit per genre
    std::vector<int> directors;
    std::string franchise;
    std::vector<int> tags; //sorted tag ids
};