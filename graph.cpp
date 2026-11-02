//
// Created by mitch on 10/31/2026.
//
#include "graph.h"
#include "similarity.h"
#include <algorithm>
#include <functional>
#include <queue>

Graph buildGraph(const std::vector<Movie> &movies, int k) {
    const int n = static_cast<int>(movies.size());
    Graph g;
    g.adj.resize(n);
    using Scored = std::pair<float, int>;

    for (int i = 0; i < n; i++) {
        std::priority_queue<Scored, std::vector<Scored>, std::greater<Scored>> best;
        for (int j = 0; j < n; ++j) {
            if ( i == j) continue;
            const float s = static_cast<float>(similarity(movies[i], movies[j]));
            if (static_cast<int>(best.size()) < k) {
                best.push({s, j});
            } else if (s > best.top().first){
                best.pop();
                best.push({s, j});
            }
        }
        while (!best.empty()) {
            g.adj[i].push_back({best.top().second, best.top().first});
            best.pop();
        }
        std::reverse(g.adj[i].begin(), g.adj[i].end()); // most similar first
    }
    return g;
}
