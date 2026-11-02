#pragma once
#include "movie.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

inline int popcount32(uint32_t x) {
    int c = 0;
    while (x) { x &= x - 1; ++c; }
    return c;
}
inline bool startsWithWord(const std::string& longer, const std::string& prefix) {
    return longer.size() > prefix.size() && longer.compare(0, prefix.size(), prefix) == 0 && longer[prefix.size()] == ' ';
}

inline double sameFranchise(const Movie& a, const Movie& b){
    const std::string& x = a.franchise;
    const std::string& y = b.franchise;
    const std::string& shorter = x.size() <= y.size() ? x : y;
    if (shorter.size() < 4) return 0.0;
    const bool same = (x == y) || startsWithWord(y, x) || startsWithWord(x, y);
    if (!same) return 0.0;
    return (a.genreMask & b.genreMask) ? 1.0 : 0.0;
}

// How much of a's tags does b cover? 0 if either movie has no tags.
// Like the genre term, extra tags on b are not penalized.
inline double tagCover(const Movie& a, const Movie& b) {
    if (a.tags.empty() || b.tags.empty()) return 0.0;
    size_t i = 0, j = 0, inter = 0;
    while (i < a.tags.size() && j < b.tags.size()) {
        if (a.tags[i] == b.tags[j]) { ++inter; ++i; ++j; }
        else if (a.tags[i] < b.tags[j]) ++i;
        else ++j;
    }
    return static_cast<double>(inter) / static_cast<double>(a.tags.size());
}

//returns a score from a scale of 0 to 1. 1 being identical on those fields and 0 for nothing alike
inline double similarity(const Movie& a, const Movie& b) {
    int inter = popcount32(a.genreMask & b.genreMask);
    int uni = popcount32((a.genreMask | b.genreMask));
    int na = popcount32(a.genreMask);
    double cover = na ? static_cast<double>(inter) / na : 0.0;
    double jaccard = uni ? static_cast<double>(inter) / uni : 0.0;
    double genre = 0.85 * cover + 0.15 * jaccard;
    double franchise = sameFranchise(a, b) * (1.0 - std::min(std::abs(a.rating - b.rating) / 5.0, 1.0));
    double tags = tagCover(a, b);
    double year = 1.0 - std::min(std::abs(a.year - b.year) / 50.0, 1.0);
    double rating = 1.0 - std::min(std::abs(a.rating - b.rating) / 5.0, 1.0);
    double runtime = 1.0 - std::min(std::abs(a.runtime - b.runtime) / 120.0, 1.0);
    return 0.3 * genre + 0.3 * franchise + 0.325 * tags + 0.025 * rating + 0.025 * year + 0.025 * runtime;
}