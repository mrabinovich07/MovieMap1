#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>

#include "graph.h"
#include "loader.h"
#include "similarity.h"
#include "tags.h"

#include <algorithm>
#include <cctype>
#include <numeric>
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Best match for the text: an exact title match beats a partial one,
// and ties go to the movie with more votes. Returns -1 if nothing matches.
int findMovie(const std::vector<Movie>& movies, const std::string& text) {
    const std::string needle = toLower(text);
    int best = -1;
    bool bestExact = false;
    for (size_t i = 0; i < movies.size(); ++i) {
        const std::string title = toLower(movies[i].title);
        if (title.find(needle) == std::string::npos) continue;
        const bool exact = (title == needle);
        if (best == -1 ||
            (exact && !bestExact) ||
            (exact == bestExact && movies[i].votes > movies[best].votes)) {
            best = static_cast<int>(i);
            bestExact = exact;
            }
    }
    return best;
}
std::string genreString(const Movie& m, const std::vector<std::string>& names) {
    std::string out;
    for (size_t b = 0; b < names.size(); ++b) {
        if (m.genreMask & (1u << b)) {
            if (!out.empty()) out += ", ";
            out += names[b];
        }
    }
    return out;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    const std::string dir = DATA_DIR;
    Dataset data = loadMovies(dir + "title.basics.tsv", dir + "title.ratings.tsv", dir + "title.crew.tsv", 100);
    std::cout << "Loaded " << data.movies.size() << " movies\n";

    size_t tagged = attachTags(data, dir + "links.csv", dir + "tags.csv", 5);
    std::cout << tagged << " movies have tags (" << data.tagNames.size() << " distinct tags kept)\n";

    std::cout << "Enter a movie title to search: ";
    std::string query;
    std::getline(std::cin, query);
    int found = findMovie(data.movies, query);
    if (found == -1) {
        std::cout << "No movie matches \"" << query << "\"\n";
        return 0;
    }
    const Movie& chosen = data.movies[found];

    // 5000 movie sample with most votes
    std::vector<size_t> order(data.movies.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return data.movies[a].votes > data.movies[b].votes;
    });
    const size_t POOL = std::min<size_t>(5000, order.size());
    std::vector<Movie> sample;
    for (size_t i = 0; i < POOL; ++i)
        sample.push_back(data.movies[order[i]]);

    // Make sure the chosen movie is in the sample
    int root = -1;
    for (size_t i = 0; i < sample.size(); ++i) {
        if (sample[i].tconst == chosen.tconst) {
            root = static_cast<int>(i);
            break;
        }
    }
    if (root == -1) {
        sample.push_back(chosen);
        root = static_cast<int>(sample.size()) - 1;
    }

    size_t poolTagged = 0;
    for (const Movie& m : sample) if (!m.tags.empty()) ++poolTagged;
    std::cout << "Pool size: " << sample.size() << " (" << poolTagged << " with tags)\n";

    Graph g = buildGraph(sample, 15);

    const Movie& r = sample[root];
    std::cout << "\nRoot: " << r.title << " (" << r.year << ") "
              << r.rating << " " << r.runtime << " min  ["
              << genreString(r, data.genreNames) << "] {" << r.franchise << "} "
              << r.tags.size() << " tags\n";
    for (const Edge& e : g.adj[root]) {
        const Movie& m = sample[e.to];
        std::cout << "  " << e.weight << "  " << m.title << " (" << m.year << ") "
                  << m.rating << " " << m.runtime << " min  ["
                  << genreString(m, data.genreNames) << "] {" << m.franchise << "} "
                  << m.tags.size() << " tags\n";
    }
    return 0;
}