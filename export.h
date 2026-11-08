#pragma once
#include "movie.h"
#include "graph.h"
#include <string>
#include <vector>
bool exportGraphJson(const std::string& path, const std::vector<Movie>& movies,
                     const std::vector<std::string>& genreNames, const Graph& graph, int root);
