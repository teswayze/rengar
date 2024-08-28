# pragma once

# define TZCNT __builtin_ctzll

# ifdef __BMI2__

# include <x86intrin.h>
# define PEXT _pext_u64
# define PDEP _pdep_u64

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

# endif