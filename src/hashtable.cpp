# include "hashtable.hpp"
# include <cstdint>
# include <vector>
# include <iostream>

bool LookupHit::operator==(const LookupHit &other) const {
	return score == other.score and move == other.move and depth == other.depth and flags == other.flags;
}

std::vector<StorageValue> hash_table;
uint64_t lookup_mask;

unsigned int put_count;
unsigned int hit_count;
unsigned int miss_count;

void ht_init(int key_length){
	put_count = 0; hit_count = 0; miss_count = 0;

	LookupHit arbitrary_value{0, 0, 0, 0};
	hash_table = std::vector(1 << key_length, StorageValue{0, arbitrary_value});
	lookup_mask = (1 << key_length) - 1;
}

std::optional<LookupHit> ht_lookup(uint64_t hash){
	auto stored = hash_table[hash & lookup_mask];
	if (hash == stored.signature){
		hit_count++;
		return std::make_optional(stored.value);
	}
	miss_count++;
	return std::nullopt;
}

void ht_put(uint64_t hash, LookupHit value){
	// Always evict
	put_count++;
	hash_table[hash & lookup_mask] = StorageValue{hash, value};
}

void ht_stats(){
	std::cout << ((lookup_mask + 1) * sizeof(StorageValue)) / (1 << 20) << "MB" << std::endl;
	std::cout << (lookup_mask + 1) << " slots" << std::endl;
	std::cout << hit_count << " hits" << std::endl;
	std::cout << miss_count << " misses" << std::endl;
	std::cout << put_count << " puts" << std::endl;
}
