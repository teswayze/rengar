# include <array>
# include <cstdint>

// Throughout this file, I use the base b = 2^(1/64)

const int log_add_exp_cutoff = 482;
const std::array<uint8_t, log_add_exp_cutoff> log_add_exp_lookup = {
		64, 64, 63, 63, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 57, 57, 56,
		56, 55, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 51, 50, 50, 49, 49,
		49, 48, 48, 47, 47, 47, 46, 46, 45, 45, 45, 44, 44, 43, 43, 43, 42,
		42, 42, 41, 41, 41, 40, 40, 39, 39, 39, 38, 38, 38, 37, 37, 37, 36,
		36, 36, 35, 35, 35, 35, 34, 34, 34, 33, 33, 33, 32, 32, 32, 32, 31,
		31, 31, 30, 30, 30, 30, 29, 29, 29, 28, 28, 28, 28, 27, 27, 27, 27,
		26, 26, 26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 24, 23, 23, 23, 23,
		22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 19, 19,
		19, 19, 19, 19, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 16, 16,
		16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14,
		14, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12,
		11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10,
		10, 10,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  8,  8,  8,  8,  8,
		8,  8,  8,  8,  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,  7,  7,  7,
		7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
		6,  6,  6,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
		5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
		4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,
		3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
		3,  3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,
		2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
		2,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1
};
// Computes log_b(b^x + b^y), assuming x >= y
constexpr int log_add_smaller_exp(int x, int y){
	const int diff = x-y;
	return x + ((diff >= log_add_exp_cutoff) ? 0 : log_add_exp_lookup[diff]);
}
// Computes log_b(b^x + b^y)
constexpr int log_add_exp(int x, int y){
	const int diff = x - y;
	const int absdiff = (diff >= 0) ? diff : -diff;
	const int maximum = (diff >= 0) ? x : y;
	const int offset = (absdiff >= log_add_exp_cutoff) ? 0 : log_add_exp_lookup[absdiff];
	return maximum + offset;
}

// Properties of quantmoid:
// - quantmoid is an odd function
// - The range is -127 to 127
// - it will never overflow if T is an int16 or bigger
// - It is a piecewise quadratic approximation to a sigmoid
template <typename T>
constexpr int quantmoid(T x){
	const T y = std::max(74 - std::abs(x), -181);
	const T z = (y * (y * 362)) / 512;
	return (x >= 0) ? (63 - z) : (z - 63);
}
