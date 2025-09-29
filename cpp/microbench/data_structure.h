#pragma once

#include <concepts>

 /*
  * Concepts describing data structures
  *
  * User-provided data structure should fit the constraints described above
  * This should be interpreted as golang interface-like entities:
  * no need to explicitly mark data structure with inheritance
  *
  * see https://en.cppreference.com/w/cpp/language/requires.html
  *
  * NB: if you want "inherit" already defined concept,
  * new concept should contain older one as conjunct
  *
  */

template <typename DS>
concept insertable_set = requires(DS ds) {
    typename DS::Key;
    typename DS::Value;
    { ds.insert(const int tid, DS::Key, DS::Value) } -> std::same_as<DS::Value>;
    { ds.find(const int tid, DS::Key) } -> std::same_as<DS::Value>;
}

template <typename DS>
concept BasicSet = insertable_set<DS> && requires(DS ds) {
    { ds.erase(const int tid, DS::Key) } -> std::same_as<DS::Value>;
}
