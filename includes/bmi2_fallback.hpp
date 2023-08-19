# pragma once

# ifdef __BMI2__

# include <x86intrin.h>
# define PEXT _pext_u64
# define PDEP _pdep_u64
# define TZCNT _tzcnt_u64

# else

# include <cstdint>

constexpr uint64_t PEXT(const uint64_t val, uint64_t mask) {
  uint64_t res = 0;
  for (uint64_t bb = 1; mask; bb += bb) {
    if ( val & mask & -mask )
      res |= bb;
    mask &= mask - 1;
  }
  return res;
}

constexpr uint64_t PDEP(const uint64_t val, uint64_t mask) {
  uint64_t res = 0;
  for (uint64_t bb = 1; mask; bb += bb) {
    if (val & bb)
      res |= mask & -mask;
    mask &= mask - 1;
  }
  return res;
}

constexpr uint8_t TZCNT(const uint64_t val) {
  uint64_t n = val & (-val);

  #define S(k) if (n >= (1ull << k)) { i += k; n >>= k; }

  uint8_t i = -(n == 0); S(32); S(16); S(8); S(4); S(2); S(1); return i;

  #undef S

  return i;
}

# endif