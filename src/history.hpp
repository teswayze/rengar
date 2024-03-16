# pragma once

# include <array>
# include <cstdint>

struct History{
	std::array<uint64_t, 256> hash_array;

	int curr_idx = 0;
	int root_idx = 0;
	int irreversible_idx = 0;

	int index_of_repetition(const uint64_t hash) const;
	History extend(const uint64_t hash);
	History make_irreversible() const;

	History wipe();
	History extend_root(const uint64_t hash);
};
