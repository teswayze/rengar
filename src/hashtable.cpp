# include "hashtable.hpp"
# include <cstdint>
# include <vector>
# include <iostream>

struct StorageValue{
	uint64_t signature;
	LookupHit value;
};

std::vector<StorageValue> hash_table;
uint64_t lookup_mask;

unsigned int put_count;
unsigned int hit_count;
unsigned int miss_count;

void ht_init(int key_length){
	put_count = 0; hit_count = 0; miss_count = 0;

	LookupHit arbitrary_value;
	hash_table = std::vector(1 << key_length, StorageValue{0, arbitrary_value});
	hash_table[0] = StorageValue{1, arbitrary_value};
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

# ifndef DOCTEST_CONFIG_DISABLE
# include "doctest.h"

TEST_CASE("Hash table miss"){
	ht_init(4);
	auto result = ht_lookup(3);
	CHECK(not result.has_value());
}

TEST_CASE("Hash table hit"){
	ht_init(4);
	LookupHit value(9, 0, 7);
	ht_put(3, value);
	auto result = ht_lookup(3);
	CHECK(result.has_value());
	CHECK(value == result.value());
}

TEST_CASE("Hash table non-collision"){
	ht_init(4);
	LookupHit value1(9, 0, 7);
	LookupHit value2(-2, 22, 3);
	ht_put(3, value1);
	ht_put(7, value2);
	auto old = ht_lookup(3);
	CHECK(old.has_value());
	CHECK(value1 == old.value());
	auto new_ = ht_lookup(7);
	CHECK(new_.has_value());
	CHECK(value2 == new_.value());
}

TEST_CASE("Hash table collision"){
	ht_init(4);
	LookupHit value1(9, 0, 7);
	LookupHit value2(-2, 22, 3);
	ht_put(3, value1);
	ht_put(19, value2);
	auto old = ht_lookup(3);
	CHECK(not old.has_value());
	auto new_ = ht_lookup(19);
	CHECK(new_.has_value());
	CHECK(value2 == new_.value());
}

TEST_CASE("Entry size"){
	CHECK(sizeof(StorageValue) == 16);
}

# endif
