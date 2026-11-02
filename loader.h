#pragma once
#include "movie.h"
#include <string>
#include <vector>

struct Dataset {
    std::vector<Movie> movies;
    std::vector<std::string> genreNames; //index = bit position in the genreMask
    std::vector<std::string> tagNames; //index = tag id;
};

Dataset loadMovies(const std::string& basicsPath, const std::string& ratingsPath, const std::string& crewPath, int minVotes);