# include "history.hpp"

bool History::is_repetition(const uint64_t hash) const{
	bool twofold = false;
	for (int idx = curr_idx - 4; idx >= irreversible_idx; idx -= 2){
		if (hash_array[idx] == hash){
			if (twofold or (idx >= root_idx)) return true;
			twofold = true;
		}
	}
	return false;
}
History History::extend(const uint64_t hash){
	hash_array[curr_idx] = hash;
	return History{hash_array, curr_idx + 1, root_idx, irreversible_idx};
}
History History::make_irreversible() const{
	return History{hash_array, curr_idx + 1, root_idx, curr_idx + 1};
}

History History::wipe(){
	return History{hash_array, 0, 0, 0};
}
History History::extend_root(const uint64_t hash){
	hash_array[curr_idx] = hash;
	return History{hash_array, curr_idx + 1, curr_idx + 1, irreversible_idx};
}
