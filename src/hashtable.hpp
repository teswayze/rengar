# include <optional>
# include <cstdint>

struct LookupHit{
	int score;
	uint16_t move;
	uint8_t depth;
	uint8_t flags;

	bool operator==(const LookupHit &other) const;
};

void ht_init(int key_length);
std::optional<LookupHit> ht_lookup(uint64_t hash);
void ht_put(uint64_t hash, LookupHit value);
void ht_stats();

struct StorageValue{
	uint64_t signature;
	LookupHit value;
};
