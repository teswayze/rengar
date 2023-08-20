# include <optional>
# include <tuple>
# include <cstdint>

// Evaluation, move, depth
using LookupHit = std::tuple<int, uint16_t, uint8_t>;

void ht_init(int key_length);
std::optional<LookupHit> ht_lookup(uint64_t hash);
void ht_put(uint64_t hash, LookupHit value);
void ht_stats();

struct StorageValue{
	uint64_t signature;
	LookupHit value;
};
