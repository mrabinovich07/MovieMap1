//kristela
#ifndef MOVIEMAPNEW_BUCKETING_H
#define MOVIEMAPNEW_BUCKETING_H

#endif //MOVIEMAPNEW_BUCKETING_H

#pragma once
#include "movie.h"
#include <string>
#include <vector>
#include <unordered_map>

// Maps a genre/decade key to a list of movie indices
using BucketMap = std::unordered_map<std::string, std::vector<int>>;

std::string getBucketKey(const Movie& m);
BucketMap buildBuckets(const std::vector<Movie>& movies);