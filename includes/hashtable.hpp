# include <optional>
# include <tuple>
# include <cstdint>

// Evaluation, move, depth
using LookupHit = std::tuple<int, uint16_t, uint8_t>;

void ht_init();
std::optional<LookupHit> ht_lookup(uint64_t hash);
void ht_put(uint64_t hash, LookupHit value);
