//
// Created by kristela

#ifndef MOVIEMAPNEW_SEARCH_H
#define MOVIEMAPNEW_SEARCH_H

#endif //MOVIEMAPNEW_SEARCH_H
#pragma once
#include "graph.h"
#include "movie.h"
#include <vector>
#include <string>

struct BenchmarkResult {
    std::string algorithmName;
    long long timeMicroseconds;
    int nodesVisited;
    double averageSimilarity;
};

std::vector<int> searchBFS(const Graph& graph, int startIndex, int maxMovies);
std::vector<int> searchDFS(const Graph& graph, int startIndex, int maxMovies);
BenchmarkResult runBenchmark(const Graph& graph, const std::vector<Movie>& movies, int startIndex, int maxMovies, bool useBFS);