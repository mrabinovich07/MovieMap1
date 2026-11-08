#include "export.h"
#include "search.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cstdint>

static std::string jsonString(const std::string& str) {
    std::ostringstream o;
    o << '"';
    for (unsigned char c : str) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if (c < 0x20) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                      << static_cast<int>(c) << std::dec << std::setfill(' ');
                } else o << static_cast<char>(c);
        }
    }
    o << '"';
    return o.str();
}


// Average elapsed time across repeated *real C++ search calls*.
// The original runBenchmark() casts to whole microseconds, which produces
// 0 for fast searches; recording nanoseconds before conversion avoids that.
static double measuredSearchMicroseconds(const Graph& graph, int root, bool useBFS, int limit) {
    constexpr int trials = 40;
    volatile std::size_t checksum = 0;
    const auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < trials; ++i) {
        const auto found = useBFS ? searchBFS(graph, root, limit) : searchDFS(graph, root, limit);
        checksum = checksum + found.size();
    }
    const auto end = std::chrono::steady_clock::now();
    (void)checksum;
    return std::chrono::duration<double, std::micro>(end - begin).count() / trials;
}

bool exportGraphJson(const std::string& path, const std::vector<Movie>& movies,
                     const std::vector<std::string>& genreNames, const Graph& graph, int root) {
    std::ofstream out(path);
    if (!out) return false;
    out << std::setprecision(12);
    out << "{\n  \"source\": \"IMDb data processed by MovieMap C++\",\n"
           "  \"sample\": false,\n  \"initialRoot\": " << root << ",\n  \"movies\": [\n";
    for (size_t i = 0; i < movies.size(); ++i) {
        const Movie& m = movies[i];
        out << "    {\"id\": " << i << ", \"imdbId\": " << jsonString(m.tconst)
            << ", \"title\": " << jsonString(m.title)
            << ", \"year\": " << m.year << ", \"runtime\": " << m.runtime
            << ", \"rating\": " << m.rating << ", \"votes\": " << m.votes
            << ", \"genres\": [";
        bool first = true;
        for (size_t b = 0; b < genreNames.size() && b < 32; ++b) {
            if (m.genreMask & (1u << b)) {
                if (!first) out << ", ";
                out << jsonString(genreNames[b]);
                first = false;
            }
        }
        out << "]}" << (i + 1 == movies.size() ? "\n" : ",\n");
    }
    out << "  ],\n  \"edges\": [\n";
    bool first = true;
    for (size_t i = 0; i < graph.adj.size(); ++i) {
        for (const Edge& e : graph.adj[i]) {
            if (!first) out << ",\n";
            out << "    {\"from\": " << i << ", \"to\": " << e.to
                << ", \"similarity\": " << e.weight << "}";
            first = false;
        }
    }
    out << "\n  ],\n  \"traversals\": [\n";
    // Generate the exact C++ traversal orders for every possible selected root.
    // This allows an offline browser interface to animate Kristela's actual
    // queue/stack implementations rather than an independently written search.
    const int maxVisited = 50;
    for (size_t rootIndex = 0; rootIndex < movies.size(); ++rootIndex) {
        auto bfs = searchBFS(graph, static_cast<int>(rootIndex), maxVisited);
        auto dfs = searchDFS(graph, static_cast<int>(rootIndex), maxVisited);
        out << "    {\"bfs\": [";
        for (size_t j = 0; j < bfs.size(); ++j) {
            if (j) out << ',';
            out << bfs[j];
        }
        out << "], \"dfs\": [";
        for (size_t j = 0; j < dfs.size(); ++j) {
            if (j) out << ',';
            out << dfs[j];
        }
        out << "]}" << (rootIndex + 1 == movies.size() ? "\n" : ",\n");
    }
    out << "  ],\n  \"benchmarkRoot\": " << root << ",\n";
    // Benchmark every possible root, not just the initial root.  This keeps
    // elapsed C++ search time and average root similarity meaningful when
    // the user selects a new movie in the offline front end.
    out << "  \"benchmarks\": [\n";
    for (size_t rootIndex = 0; rootIndex < movies.size(); ++rootIndex) {
        auto bfsBenchmark = runBenchmark(graph, movies, static_cast<int>(rootIndex), maxVisited, true);
        auto dfsBenchmark = runBenchmark(graph, movies, static_cast<int>(rootIndex), maxVisited, false);
        out << "    {\"bfs\": {\"nodesVisited\": " << bfsBenchmark.nodesVisited
            << ", \"timeMicroseconds\": " << measuredSearchMicroseconds(graph, static_cast<int>(rootIndex), true, maxVisited)
            << ", \"averageSimilarity\": " << bfsBenchmark.averageSimilarity
            << "}, \"dfs\": {\"nodesVisited\": " << dfsBenchmark.nodesVisited
            << ", \"timeMicroseconds\": " << measuredSearchMicroseconds(graph, static_cast<int>(rootIndex), false, maxVisited)
            << ", \"averageSimilarity\": " << dfsBenchmark.averageSimilarity
            << "}}" << (rootIndex + 1 == movies.size() ? "\n" : ",\n");
    }
    out << "  ]\n}\n";
    return out.good();
}
