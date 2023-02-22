# include "board.hpp"
# include "history.hpp"
# include "parse_format.hpp"

History extend_history(const Board board, const History history){
	return std::make_shared<ListNode<Board>>(board, history);
}

bool exists_in_history(const Board board, const History history){
	// Only check every other to allow triangulation e.g.
	if (history == NULL) { return false; }
	if (history->tail == NULL) { return false; }
	if (board == history->tail->head) { return true; }
	return exists_in_history(board, history->tail->tail);
}

Variation prepend_to_variation(const Move move, const Variation variation){
	return std::make_shared<ListNode<Move>>(move, variation);
}

std::string show_variation_helper(const Variation variation, std::string prefix){
	if (variation == NULL) {return prefix;}
	return show_variation_helper(variation->tail, prefix + " " + format_move_xboard(variation->head));
}

std::string show_variation(const Variation variation){
	return show_variation_helper(variation, "");
}
