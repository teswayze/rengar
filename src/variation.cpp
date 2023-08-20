# include "variation.hpp"
# include <algorithm>

VariationView VariationView::prepend(const Move move) const{
	auto prev = location - maxsize - 1;
	*prev = move;
	std::copy(location, location + length, prev + 1);
	return VariationView{prev, length + 1, maxsize + 1};
}
VariationView VariationView::fresh_branch() const{
	return VariationView{location + maxsize, 0, maxsize - 1};
}
VariationView VariationView::copy_branch() const{
	if (length) std::copy(location + 1, location + length, location + maxsize);
	return VariationView{location + maxsize, length - 1, maxsize - 1};
}
VariationView VariationView::nullify() const{
	return VariationView{location, 0, maxsize};
}
VariationView VariationView::singleton(const Move move) const{
	*location = move;
	return VariationView{location, 1, maxsize};
}
Move VariationView::head() const{
	return *location;
}
