# pragma once

# include <array>
# include <cstdint>

struct History{
	std::array<uint64_t, 256> hash_array;
	int root_idx = 0;

	void wipe();
	void extend_root(const uint64_t hash);
};


struct HistoryView{
	History &history;
	int curr_idx;
	int irreversible_idx;

	int index_of_repetition(const uint64_t hash, bool twofold = false) const;
	HistoryView extend(const uint64_t hash);
	HistoryView make_irreversible() const;

	void operator=(HistoryView hv);
};


HistoryView take_view(History &history);
