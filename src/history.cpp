# include "history.hpp"

int HistoryView::index_of_repetition(const uint64_t hash, bool twofold = false) const{
	for (int idx = curr_idx - 4; idx >= irreversible_idx; idx -= 2){
		if (history.hash_array[idx] == hash){
			if (twofold or (idx >= history.root_idx)) return idx;
			twofold = true;
		}
	}
	return -1;
}
HistoryView HistoryView::extend(const uint64_t hash){
	history.hash_array[curr_idx] = hash;
	return HistoryView{history, curr_idx + 1, irreversible_idx};
}
HistoryView HistoryView::make_irreversible() const{
	return HistoryView{history, curr_idx + 1, curr_idx + 1};
}
void HistoryView::operator=(HistoryView hv){
	this->history = hv.history;
	this->curr_idx = hv.curr_idx;
	this->irreversible_idx = hv.irreversible_idx;
}


void History::wipe(){
	root_idx = 0;
}
void History::extend_root(const uint64_t hash){
	hash_array[root_idx] = hash;
	root_idx++;
}

HistoryView take_view(History &history){
	return HistoryView{history, history.root_idx, 0};
}
