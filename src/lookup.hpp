# pragma once

# include <array>

template <class table_type, size_t size>
constexpr auto lookup_table(table_type func(size_t)){
	std::array<table_type, size> arr {};
	for (size_t i = 0; i < size; i++){
		arr[i] = func(i);
	}
	return arr;
}


template <class table_type, size_t size1, size_t size2>
constexpr auto lookup_table_2d(table_type func(size_t, size_t)){
	std::array<std::array<table_type, size2>, size1> arr {};
	for (size_t i = 0; i < size1; i++){
		for (size_t j = 0; j < size2; j++){
			arr[i][j] = func(i, j);
		}
	}
	return arr;
}


template <class table_type, size_t size1, size_t size2, size_t size3>
constexpr auto lookup_table_3d(table_type func(size_t, size_t, size_t)){
	std::array<std::array<std::array<table_type, size3>, size2>, size1> arr {};
	for (size_t i = 0; i < size1; i++){
		for (size_t j = 0; j < size2; j++){
			for (size_t k = 0; k < size3; k++){
				arr[i][j][k] = func(i, j, k);
			}
		}
	}
	return arr;
}
