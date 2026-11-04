//kristela
#include "search.h"
#include "similarity.h"
#include <queue>
#include <stack>
#include <chrono>

std::vector<int> searchBFS(const Graph& graph, int startIndex, int maxMovies) {
    std::vector<int> results;
    // Using a boolean vector is O(1) and much faster than unordered_set
    std::vector<bool> visited(graph.adj.size(), false);
    std::queue<int> q;

    q.push(startIndex);
    visited[startIndex] = true;

    while (!q.empty() && results.size() < maxMovies) {
        int current = q.front();
        q.pop();

        if (current != startIndex) {
            results.push_back(current);
        }

        // graph.adj is a vector of vectors of Edges
        for (const Edge& edge : graph.adj[current]) {
            if (!visited[edge.to]) {
                visited[edge.to] = true;
                q.push(edge.to);
            }
        }
    }
    return results;
}

std::vector<int> searchDFS(const Graph& graph, int startIndex, int maxMovies) {
    std::vector<int> results;
    std::vector<bool> visited(graph.adj.size(), false);
    std::stack<int> s;

    s.push(startIndex);

    while (!s.empty() && results.size() < maxMovies) {
        int current = s.top();
        s.pop();

        if (!visited[current]) {
            visited[current] = true;
            
            if (current != startIndex) {
                results.push_back(current);
            }

            // Push in reverse order so we visit the most similar neighbor first 
            // (Mitchell sorted them most similar first in graph.cpp[cite: 11])
            const auto& neighbors = graph.adj[current];
            for (auto rit = neighbors.rbegin(); rit != neighbors.rend(); ++rit) {
                if (!visited[rit->to]) {
                    s.push(rit->to);
                }
            }
        }
    }
    return results;
}

BenchmarkResult runBenchmark(const Graph& graph, const std::vector<Movie>& movies, int startIndex, int maxMovies, bool useBFS) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<int> foundMovies;
    if (useBFS) {
        foundMovies = searchBFS(graph, startIndex, maxMovies);
    } else {
        foundMovies = searchDFS(graph, startIndex, maxMovies);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    double totalSimilarity = 0.0;
    const Movie& startMovie = movies[startIndex];
    
    for (int id : foundMovies) {
        // Calling Mitchell's similarity function[cite: 7]
        totalSimilarity += similarity(startMovie, movies[id]);
    }
    
    double avgSim = foundMovies.empty() ? 0.0 : (totalSimilarity / foundMovies.size());

    return {
        useBFS ? "BFS" : "DFS",
        duration.count(),
        static_cast<int>(foundMovies.size()),
        avgSim
    };
}