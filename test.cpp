//kristela
#include "catch_amalgamated.hpp"
#include "search.h"
#include "graph.h"
#include "movie.h"

TEST_CASE("BFS and DFS Traversal Logic", "[search]") {
    // Setup Dummy Data
    std::vector<Movie> movies(5);
    movies[0].title = "Root";
    movies[1].title = "Neighbor 1";
    movies[2].title = "Neighbor 2";
    movies[3].title = "Deep 1";
    movies[4].title = "Deep 2";

    Graph g;
    g.adj.resize(5);
    // Root(0) connects to 1 and 2
    g.adj[0].push_back({1, 0.9f});
    g.adj[0].push_back({2, 0.8f});
    // Node 1 connects to 3
    g.adj[1].push_back({3, 0.7f});
    // Node 2 connects to 4
    g.adj[2].push_back({4, 0.6f});
    // Node 4 cycles back to root to test visited set prevention
    g.adj[4].push_back({0, 0.5f});

    SECTION("BFS visits level-by-level") {
        std::vector<int> bfsNodes = searchBFS(g, 0, 5);
        REQUIRE(bfsNodes.size() == 4); // Excludes root
        REQUIRE(bfsNodes[0] == 1); // Level 1
        REQUIRE(bfsNodes[1] == 2); // Level 1
        REQUIRE(bfsNodes[2] == 3); // Level 2
        REQUIRE(bfsNodes[3] == 4); // Level 2
    }

    SECTION("DFS visits depth-first") {
        std::vector<int> dfsNodes = searchDFS(g, 0, 5);
        REQUIRE(dfsNodes.size() == 4);
        // Due to the reverse-iterator stack push in search.cpp, it visits node 1 first
        REQUIRE(dfsNodes[0] == 1);
        REQUIRE(dfsNodes[1] == 3);
        REQUIRE(dfsNodes[2] == 2);
        REQUIRE(dfsNodes[3] == 4);
    }
}