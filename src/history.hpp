# pragma once

# include <memory>
# include <string>

# include "move.hpp"

template <typename T>
struct ListNode{
	const T head;
	const int length;
	const std::shared_ptr<const ListNode<T>> tail;

	ListNode(const T h, const std::shared_ptr<const ListNode<T>> t) :
		head(h), length(t == NULL ? 0 : (t->length + 1)), tail(t) { }
};

using History = std::shared_ptr<const ListNode<Board>>;
using Variation = std::shared_ptr<const ListNode<Move>>;

History extend_history(const Board board, const History history);
bool exists_in_history(const Board board, const History history);
Variation prepend_to_variation(const Move move, const Variation variation);
std::string show_variation(const Variation variation);
