#pragma once

#include <sstream>
#include <string>

#include "catch2.h"

struct HeapAllocator {
  static inline void* alloc(uint64_t size) { return ::malloc(size); }
  static inline void* realloc(void* ptr, uint64_t size) {
    return ::realloc(ptr, size);
  }
  static inline void free(void* ptr) { ::free(ptr); }
};

using natural_ptr = bigmath::natural<HeapAllocator>*;

struct Z {
  natural_ptr nat;

  explicit Z(natural_ptr n) : nat(n) {}

  Z(std::initializer_list<u64> words) {
    nat =
        bigmath::nat_new_words<HeapAllocator>(words.begin(), (u32)words.size());
  }
  Z(Z&& other) noexcept : nat(std::exchange(other.nat, nullptr)) {}

  ~Z() noexcept {
    if (nat) {
      bigmath::nat_free(nat);
    }
  }

  Z operator+(const Z& other) const {
    u32 max_size = nat->places_count < other.nat->places_count
                       ? other.nat->places_count
                       : nat->places_count;
    auto result = bigmath::nat_clone(nat, max_size + 1);
    result = bigmath::nat_add_nat(result, other.nat);
    return Z{result};
  }

  Z operator*(u64 word) const {
    auto result = bigmath::nat_mul_word<HeapAllocator>(nullptr, nat, word);
    return Z{result};
  }

  Z operator+(u64 word) const {
    auto result = bigmath::nat_clone(nat, nat->places_count + 1);
    result = bigmath::nat_add_word(result, word);
    return Z{result};
  }

  bool operator==(const Z& other) const {
    auto other_nat = other.nat;

    // FIXME: implement actual routine in bigmath.
    if (nat->places_count != other_nat->places_count) {
      return false;
    }

    for (u32 i = 0; i < bigmath::place_t::size * nat->places_count; ++i) {
      if (nat->words[i] != other_nat->words[i]) {
        return false;
      }
    }

    return true;
  }
};

template <>
struct Catch::StringMaker<Z> {
  static std::string convert(const Z& value) {
    std::ostringstream ss;

    ss << "Z{";
    for (u32 i = 0; i < bigmath::place_t::size * value.nat->places_count; ++i) {
      if (i != 0) {
        ss << ", ";
      }
      ss << value.nat->words[i];
    }
    ss << "}";

    return ss.str();
  }
};

class ZMatcher : public Catch::MatcherBase<Z> {
 public:
  ZMatcher(const ZMatcher& m) : expected(bigmath::nat_clone(m.expected.nat)) {}
  ZMatcher(Z&& _expected) : expected(std::move(_expected)) {}

  bool match(const Z& value) const override {
    auto nat = expected.nat;

    // FIXME: implement actual routine in bigmath.
    if (nat->places_count != value.nat->places_count) {
      return false;
    }

    for (u32 i = 0; i < bigmath::place_t::size * nat->places_count; ++i) {
      if (nat->words[i] != value.nat->words[i]) {
        return false;
      }
    }

    return true;
  }

  std::string describe() const override {
    return "== " + Catch::StringMaker<Z>::convert(expected);
  }

  Z expected;
};

inline ZMatcher ZEquals(Z&& expected) { return ZMatcher{std::move(expected)}; }
inline ZMatcher ZEquals(std::initializer_list<u64> places) {
  return ZMatcher{Z(places)};
}
