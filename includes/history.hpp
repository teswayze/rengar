# pragma once

# include <array>

struct History2{
	std::array<uint64_t, 256> hash_array;

	int curr_idx = 0;
	int root_idx = 0;
	int irreversible_idx = 0;

	bool is_repetition(const uint64_t hash) const;
	History2 extend(const uint64_t hash);
	History2 make_irreversible() const;

	History2 wipe();
	History2 extend_root(const uint64_t hash);
};
