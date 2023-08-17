# pragma once

# ifdef __BMI__

# include <x86intrin.h>
# define BLSR __blsr_u64

# else

# include <cstdint>
constexpr uint64_t BLSR(const uint64_t x){ return x & (x-1); }

# endif