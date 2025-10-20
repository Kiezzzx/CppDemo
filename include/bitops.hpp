#pragma once
#include <cstdint>

inline void set_bit(uint32_t& reg, unsigned n){ reg |=  (1u << n); }
inline void clr_bit(uint32_t& reg, unsigned n){ reg &= ~(1u << n); }
inline void tog_bit(uint32_t& reg, unsigned n){ reg ^=  (1u << n); }
inline bool tst_bit(uint32_t  reg, unsigned n){ return (reg >> n) & 1u; }
