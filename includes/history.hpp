# pragma once

# include <memory>
# include <array>

# include "board.hpp"

template <typename T>
struct ListNode{
	const T head;
	const std::shared_ptr<const ListNode<T>> tail;

	ListNode(const T h, const std::shared_ptr<const ListNode<T>> t) :
		head(h), tail(t) { }
};

using History = std::shared_ptr<const ListNode<uint64_t>>;

History extend_history(const Board &board, const History history);
bool exists_in_history(const Board &board, const History history);
History remove_single_repetitions(const History history);
History remove_hash_from_history(const History history, const Board &board);


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
