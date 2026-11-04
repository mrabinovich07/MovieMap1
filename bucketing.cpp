//kristela

#include "bucketing.h"

std::string getBucketKey(const Movie& m) {
    // Calculate the decade (e.g., 1994 -> 1990)
    int decade = (m.year / 10) * 10;

    // Isolate the primary genre by extracting the lowest set bit in the mask
    // If the movie has no genre (genreMask is 0), it defaults to 0
    uint32_t primaryGenreMask = m.genreMask & -(int32_t)m.genreMask;

    // Creates a key like "4_1990s" where 4 represents the bitmask of the primary genre
    return std::to_string(primaryGenreMask) + "_" + std::to_string(decade) + "s";
}

BucketMap buildBuckets(const std::vector<Movie>& movies) {
    BucketMap buckets;
    for (int i = 0; i < static_cast<int>(movies.size()); ++i) {
        std::string key = getBucketKey(movies[i]);
        buckets[key].push_back(i);
    }
    return buckets;
}