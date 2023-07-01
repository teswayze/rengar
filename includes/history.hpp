# pragma once

# include <memory>

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
