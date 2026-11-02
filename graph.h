#pragma once
#include "movie.h"
#include <vector>

struct Edge {
    int to;
    float weight;
};

struct Graph {
    std::vector<std::vector<Edge>> adj;
};

Graph buildGraph(const std::vector<Movie>& movies, int k);