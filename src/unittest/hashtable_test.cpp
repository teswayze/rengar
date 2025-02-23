# include "../external/doctest.h"
# include "../hashtable.hpp"

TEST_CASE("Hash table miss"){
	ht_init(4);
	auto result = ht_lookup(3);
	CHECK(not result.has_value());
}

TEST_CASE("Hash table hit"){
	ht_init(4);
	LookupHit value{9, 0, 7, 1};
	ht_put(3, value);
	auto result = ht_lookup(3);
	CHECK(result.has_value());
	CHECK(value == result.value());
}

TEST_CASE("Hash table non-collision"){
	ht_init(4);
	LookupHit value1{9, 0, 7, 1};
	LookupHit value2{-2, 22, 3, 2};
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
	LookupHit value1{9, 0, 7, 1};
	LookupHit value2{-2, 22, 3, 2};
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
