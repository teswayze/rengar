# pragma once

# include <array>
# include "board.hpp"

const int max_var_length = 90;
const int triangular_var_array_size = (max_var_length * (max_var_length + 1)) / 2;
using VariationWorkspace = std::array<Move, triangular_var_array_size>;

struct VariationView{
	Move *location;
	size_t length;
	size_t maxsize;
	
	VariationView(VariationWorkspace &var_space) : location(var_space.data()), length(0), maxsize(max_var_length) { };
	VariationView(Move *location_, size_t length_, size_t maxsize_) : location(location_), length(length_), maxsize(maxsize_) { };
	
	VariationView prepend(const Move move) const;
	VariationView fresh_branch() const;
	VariationView copy_branch() const;
	VariationView nullify() const;
	VariationView singleton(const Move move) const;
	Move head() const;
};
