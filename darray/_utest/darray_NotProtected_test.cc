/******************************************************************************
 *  This is free and unencumbered software released into the public domain.
 *
 *  Anyone is free to copy, modify, publish, use, compile, sell, or
 *  distribute this software, either in source code form or as a compiled
 *  binary, for any purpose, commercial or non-commercial, and by any
 *  means.
 *
 *  In jurisdictions that recognize copyright laws, the author or authors
 *  of this software dedicate any and all copyright interest in the
 *  software to the public domain. We make this dedication for the benefit
 *  of the public at large and to the detriment of our heirs and
 *  successors. We intend this dedication to be an overt act of
 *  relinquishment in perpetuity of all present and future rights to this
 *  software under copyright law.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For more information, please refer to <https://unlicense.org>
 */

#include "darray.hpp"
#include "gtest.h"

#include <algorithm>
#include <expected>
#include <iterator>
#include <optional>

using CppPlay::darray;
using CppPlay::error;

using std::ranges::sort;
using std::ranges::transform;

using std::expected;
using std::function;
using std::optional;
using std::span;
using std::string;
using std::unexpected;

//=============================================================================
// Helper Classes and Functions
//=============================================================================
class CopyOnlyObject {
public:
  string m_guts;
  CopyOnlyObject() : m_guts{"CopyOnlyObjectDefaultGuts"} {};
  explicit CopyOnlyObject(string guts) : m_guts{guts} {};
  CopyOnlyObject(const CopyOnlyObject &other) {
    // cout << "CopyOnlyObject copy constructor\n";
    m_guts = other.m_guts;
  };
  CopyOnlyObject &operator=(const CopyOnlyObject &other) {
    // cout << "CopyOnlyObject copy assignment operator\n";
    m_guts = other.m_guts;
    return *this;
  };
  CopyOnlyObject(CopyOnlyObject &&) = delete;
  CopyOnlyObject &operator=(CopyOnlyObject &&) = delete;
  bool operator==(const CopyOnlyObject &other) const {
    return other.m_guts == m_guts;
  };
};
void PrintTo(const CopyOnlyObject &m, std::ostream *os) {
  *os << m.m_guts << std::endl;
}

class MoveOnlyObject {
public:
  string m_guts;
  MoveOnlyObject() : m_guts{"MoveOnlyObjectDefaultGuts"} {};
  explicit MoveOnlyObject(string guts) : m_guts{guts} {};
  MoveOnlyObject(MoveOnlyObject &&other) {
    // cout << "MoveOnlyObject move constructor\n";
    m_guts = other.m_guts;
  };
  MoveOnlyObject &operator=(MoveOnlyObject &&other) {
    // cout << "MoveOnlyObject move assignment operator\n";
    m_guts = other.m_guts;
    return *this;
  };
  MoveOnlyObject(MoveOnlyObject &) = delete;
  MoveOnlyObject &operator=(MoveOnlyObject &) = delete;
  bool operator==(const MoveOnlyObject &other) const {
    return other.m_guts == m_guts;
  };
};
void PrintTo(const MoveOnlyObject &m, std::ostream *os) {
  *os << m.m_guts << std::endl;
}

template <typename T, typename U> struct TestSpecPushBack {
  darray<T> &darray_obj;
  optional<darray<T> *> p_darray_obj_2_optional;
  U &&element;
  size_t expected_size;
  size_t expected_capacity;
  span<T> expected_values;

  class Builder {
    darray<T> &_darray_obj;
    optional<darray<T> *> _p_darray_obj_2_optional{};
    U &&_element;
    size_t _expected_size{};
    size_t _expected_capacity{};
    span<T> _expected_values{};

  public:
    Builder(darray<T> &darray_obj, U &&element, size_t expected_size,
            size_t expected_capacity, span<T> expected_values)
        : _darray_obj{darray_obj}, _element{element},
          _expected_size{expected_size}, _expected_capacity{expected_capacity},
          _expected_values{expected_values} {}
    auto Darray2(darray<T> *p_darray_obj_2) -> Builder {
      _p_darray_obj_2_optional.emplace(p_darray_obj_2);
      return *this;
    }
    [[nodiscard]] auto build() noexcept -> TestSpecPushBack<T, U> {
      TestSpecPushBack<T, U> built = {
          _darray_obj,    _p_darray_obj_2_optional, _element,
          _expected_size, _expected_capacity,       _expected_values};
      return built;
    }
  };
};
template <typename T, typename U>
auto test_spec_push_back_to_void =
    []([[maybe_unused]] const TestSpecPushBack<T, U> *p_spec) {
      return expected<void, error>{};
    };
template <typename T, typename U>
auto tester_push_back_base(function<expected<size_t, error>()> push_back_action,
                           darray<T> &darray_obj, size_t expected_size,
                           size_t expected_capacity, span<T> expected_values)
    -> expected<void, error> {
  return push_back_action()
      .and_then([&](size_t size) -> expected<void, error> {
        EXPECT_EQ(size, expected_size);
        EXPECT_EQ((size_t)expected_size, darray_obj.size().value());
        EXPECT_EQ(false, darray_obj.is_empty().value());
        EXPECT_EQ((size_t)expected_size, darray_obj.pod().size());
        EXPECT_EQ(false, darray_obj.pod().is_empty());
        return {};
      })
      .and_then([&]() -> expected<void, error> {
        return darray_obj.capacity().and_then(
            [&](size_t capacity) -> expected<void, error> {
              EXPECT_EQ((size_t)expected_capacity, capacity);
              EXPECT_EQ((size_t)expected_capacity,
                        darray_obj.capacity().value());
              EXPECT_EQ((size_t)expected_capacity, darray_obj.pod().capacity());
              return {};
            });
      })
      .and_then([&]() -> expected<void, error> {
        size_t idx = 0;
        for (auto &value : expected_values) {
          EXPECT_EQ(value, darray_obj[idx]);
          EXPECT_EQ(value, *(darray_obj.at(idx++).value()));
        }
        return {};
      });
};
template <typename T, typename U>
auto tester_push_back(const TestSpecPushBack<T, U> *const p_spec)
    -> expected<const TestSpecPushBack<T, U> *const, error> {
  function<expected<size_t, error>()> action =
      [&]() -> expected<size_t, error> {
    return p_spec->darray_obj.push_back(std::forward<U>(p_spec->element));
  };
  return tester_push_back_base<T, U>(
             action, p_spec->darray_obj, p_spec->expected_size,
             p_spec->expected_capacity, p_spec->expected_values)
      .and_then([&]() {
        return expected<const TestSpecPushBack<T, U> *const, error>{p_spec};
      });
}
template <typename T, typename U>
auto tester_push_back_2(const TestSpecPushBack<T, U> *const p_spec)
    -> expected<const TestSpecPushBack<T, U> *const, error> {
  if (false == p_spec->p_darray_obj_2_optional.has_value()) {
    return unexpected{error{"test_push_back_2 called without darray 2"}};
  }
  function<expected<size_t, error>()> action =
      [&]() -> expected<size_t, error> {
    return p_spec->p_darray_obj_2_optional.value()->push_back_2(
        std::forward<U>(p_spec->element));
  };
  return tester_push_back_base<T, U>(
             action, *(p_spec->p_darray_obj_2_optional.value()),
             p_spec->expected_size, p_spec->expected_capacity,
             p_spec->expected_values)
      .and_then([&]() {
        return expected<const TestSpecPushBack<T, U> *const, error>{p_spec};
      });
}

template <typename T, typename U>
auto tester_insert(size_t index, darray<T> &darray_obj, U &&element,
                   size_t expected_size, size_t expected_capacity,
                   span<T> expected_values) -> expected<void, error> {
  return darray_obj.insert(std::forward<U>(element), index)
      .and_then([&](size_t size) -> expected<void, error> {
        EXPECT_EQ(size, expected_size);
        EXPECT_EQ((size_t)expected_size, darray_obj.size().value());
        EXPECT_EQ(false, darray_obj.is_empty().value());
        EXPECT_EQ((size_t)expected_size, darray_obj.pod().size());
        EXPECT_EQ(false, darray_obj.pod().is_empty());
        return {};
      })
      .and_then([&]() -> expected<void, error> {
        return darray_obj.capacity().and_then(
            [&](size_t capacity) -> expected<void, error> {
              EXPECT_EQ((size_t)expected_capacity, capacity);
              EXPECT_EQ((size_t)expected_capacity,
                        darray_obj.capacity().value());
              EXPECT_EQ((size_t)expected_capacity, darray_obj.pod().capacity());
              return {};
            });
      })
      .and_then([&]() -> expected<void, error> {
        size_t idx = 0;
        for (auto &value : expected_values) {
          EXPECT_EQ(value, darray_obj[idx]);
          EXPECT_EQ(value, *(darray_obj.at(idx++).value()));
        }
        return {};
      });
};

template <typename T>
auto tester_pop_back(darray<T> &darray_obj, T expected_element,
                     size_t expected_size, size_t expected_capacity,
                     span<T> expected_values) -> expected<void, error> {
  return darray_obj.pop_back()
      .and_then([&](T &&element) -> expected<void, error> {
        EXPECT_EQ(expected_element, element);
        EXPECT_EQ((size_t)expected_size, darray_obj.size().value());
        EXPECT_EQ((0 == expected_size), darray_obj.is_empty().value());
        EXPECT_EQ((size_t)expected_size, darray_obj.pod().size());
        EXPECT_EQ((0 == expected_size), darray_obj.pod().is_empty());
        return {};
      })
      .and_then([&]() -> expected<void, error> {
        return darray_obj.capacity().and_then(
            [&](size_t capacity) -> expected<void, error> {
              EXPECT_EQ((size_t)expected_capacity, capacity);
              EXPECT_EQ((size_t)expected_capacity,
                        darray_obj.capacity().value());
              EXPECT_EQ((size_t)expected_capacity, darray_obj.pod().capacity());
              return {};
            });
      })
      .and_then([&]() -> expected<void, error> {
        size_t idx = 0;
        for (auto &value : expected_values) {
          EXPECT_EQ(value, darray_obj[idx]);
          EXPECT_EQ(value, *(darray_obj.at(idx++).value()));
          // EXPECT_EQ(value,*(darray_obj.cat(idx++).value()));
        }
        return {};
      });
};

template <typename T>
auto tester_extract(size_t index_to_erase, darray<T> &darray_obj,
                    T expected_element, size_t expected_size,
                    size_t expected_capacity, span<T> expected_values)
    -> expected<void, error> {
  return darray_obj.extract(index_to_erase)
      .and_then([&](T &&element) -> expected<void, error> {
        EXPECT_EQ(expected_element, element);
        EXPECT_EQ((size_t)expected_size, darray_obj.size().value());
        EXPECT_EQ((0 == expected_size), darray_obj.is_empty().value());
        EXPECT_EQ((size_t)expected_size, darray_obj.pod().size());
        EXPECT_EQ((0 == expected_size), darray_obj.pod().is_empty());
        return {};
      })
      .and_then([&]() -> expected<void, error> {
        return darray_obj.capacity().and_then(
            [&](size_t capacity) -> expected<void, error> {
              EXPECT_EQ((size_t)expected_capacity, capacity);
              EXPECT_EQ((size_t)expected_capacity,
                        darray_obj.capacity().value());
              EXPECT_EQ((size_t)expected_capacity, darray_obj.pod().capacity());
              return {};
            });
      })
      .and_then([&]() -> expected<void, error> {
        size_t idx = 0;
        for (auto &value : expected_values) {
          EXPECT_EQ(value, darray_obj[idx]);
          EXPECT_EQ(value, *(darray_obj.at(idx++).value()));
          // EXPECT_EQ(value,*(darray_obj.cat(idx++).value()));
        }
        return {};
      });
};

//=============================================================================
// Tests
//=============================================================================
TEST(darray, constructDefault) {

  darray<int> darray_obj{};
  auto _ = darray_obj.capacity()
               .and_then([](size_t capacity) -> expected<size_t, error> {
                 EXPECT_EQ((size_t)8, capacity);
                 return {capacity};
               })
               .or_else([](error err) -> expected<size_t, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

TEST(darray, constructReserve) {

  darray<int> darray_obj = darray<int>::builder{}.capacity(10).build();
  auto _ = darray_obj.capacity()
               .and_then([](size_t capacity) -> expected<size_t, error> {
                 EXPECT_EQ((size_t)10, capacity);
                 return {capacity};
               })
               .or_else([](error err) -> expected<size_t, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

#ifdef TEST_COMPILATION_FAILURE
TEST(darray, constructCopyMoveOnly) {

  // when enabled should fail with compilation error that
  // MoveOnlyObject copy assignment operator (operator=) is unavailable/deleted

  darray<MoveOnlyObject> darray_obj =
      darray<MoveOnlyObject>::builder{}.capacity(2).build();

  auto test_1 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"}};
    MoveOnlyObject element{"1"};
    TestSpecPushBack<MoveOnlyObject, MoveOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        1,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(MoveOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<MoveOnlyObject, MoveOnlyObject &>);
  };

  auto _ = test_1()
               .and_then([&]() -> expected<void, error> {
                 darray copy{darray_obj}; // can't copy move only object, so
                                          // should fail compilation here
                 return {};
               })
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}
#endif // TEST_COMPILATION_FAILURE

TEST(darray, copyConstcopy) {

  darray<CopyOnlyObject> darray_obj =
      darray<CopyOnlyObject>::builder{}.capacity(2).build();
  darray<CopyOnlyObject> darray_obj_2 = darray_obj;

  auto test_1 = [&]() -> expected<void, error> {
    CopyOnlyObject element{"1"};
    CopyOnlyObject expected_elements[]{CopyOnlyObject{"1"}};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        optional{&darray_obj_2},
        element,
        1,
        2,
        span{expected_elements,
             sizeof(expected_elements) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<CopyOnlyObject, CopyOnlyObject &>)
        .and_then(
            test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto test_2 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"},
                                        CopyOnlyObject{"2"}};
    CopyOnlyObject element{"2"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        optional{&darray_obj_2},
        element,
        2,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<CopyOnlyObject, CopyOnlyObject &>)
        .and_then(
            test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto test_3 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"}};
    CopyOnlyObject element{"3"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        optional{&darray_obj_2},
        element,
        3,
        4,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<CopyOnlyObject, CopyOnlyObject &>)
        .and_then(
            test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto test_4_general =
      [](darray<CopyOnlyObject> &darray_obj) -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"},
        CopyOnlyObject{"4"}};
    CopyOnlyObject element{"4"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        4,
        4,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto test_5_general =
      [](darray<CopyOnlyObject> &darray_obj) -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"},
        CopyOnlyObject{"4"}, CopyOnlyObject{"5"}};
    CopyOnlyObject element{"5"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        5,
        8,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };

  auto _ = test_1()
               .and_then(test_2)
               .and_then(test_3)
               // copy constructor
               .and_then([&]() -> expected<void, error> {
                 darray<CopyOnlyObject> copy{darray_obj};
                 return test_4_general(copy);
               })
               .and_then([&]() -> expected<void, error> {
                 EXPECT_EQ((size_t)3, darray_obj.pod().size());
                 EXPECT_EQ((size_t)4, darray_obj.pod().capacity());
                 EXPECT_EQ(false, darray_obj.pod().is_empty());
                 return test_4_general(darray_obj);
               })
               // copy assignment operator
               .and_then([&]() -> expected<void, error> {
                 darray<CopyOnlyObject> copy = darray_obj;
                 return test_5_general(copy);
               })
               .and_then([&]() -> expected<void, error> {
                 EXPECT_EQ((size_t)4, darray_obj.pod().size());
                 EXPECT_EQ((size_t)4, darray_obj.pod().capacity());
                 EXPECT_EQ(false, darray_obj.pod().is_empty());
                 return test_5_general(darray_obj);
               })
               // ensure failure registered for unexpected
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

TEST(darray, pushPopBackMove) {

  darray<MoveOnlyObject> darray_obj =
      darray<MoveOnlyObject>::builder{}.capacity(2).build();
  darray<MoveOnlyObject> darray_obj_2 =
      darray<MoveOnlyObject>::builder{}.capacity(2).build();

  auto push_back_1 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"}};
    TestSpecPushBack<MoveOnlyObject, MoveOnlyObject &&> spec = {
        darray_obj,
        optional{&darray_obj_2},
        MoveOnlyObject{"1"},
        1,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<MoveOnlyObject, MoveOnlyObject &&>)
        .and_then(
            test_spec_push_back_to_void<MoveOnlyObject, MoveOnlyObject &&>);
  };
  auto push_back_2 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"},
                                        MoveOnlyObject{"2"}};
    TestSpecPushBack<MoveOnlyObject, MoveOnlyObject &&> spec = {
        darray_obj,
        optional{&darray_obj_2},
        MoveOnlyObject{"2"},
        2,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<MoveOnlyObject, MoveOnlyObject &&>)
        .and_then(
            test_spec_push_back_to_void<MoveOnlyObject, MoveOnlyObject &&>);
  };
  auto push_back_3 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"1"}, MoveOnlyObject{"2"}, MoveOnlyObject{"3"}};
    TestSpecPushBack<MoveOnlyObject, MoveOnlyObject &&> spec = {
        darray_obj,
        optional{&darray_obj_2},
        MoveOnlyObject{"3"},
        3,
        4,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec)
        .and_then(tester_push_back_2<MoveOnlyObject, MoveOnlyObject &&>)
        .and_then(
            test_spec_push_back_to_void<MoveOnlyObject, MoveOnlyObject &&>);
  };

  auto pop_back_1 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"},
                                        MoveOnlyObject{"2"}};
    return tester_pop_back(darray_obj, MoveOnlyObject{"3"}, 2, 2,
                           span{expected_values, sizeof(expected_values) /
                                                     sizeof(MoveOnlyObject)});
  };
  auto pop_back_2 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"}};
    return tester_pop_back(darray_obj, MoveOnlyObject{"2"}, 1, 2,
                           span{expected_values, sizeof(expected_values) /
                                                     sizeof(MoveOnlyObject)});
  };
  auto pop_back_3 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[1]; // "1" to silence compiler
    return tester_pop_back(darray_obj, MoveOnlyObject{"1"}, 0, 2,
                           span{expected_values, 0});
  };
  auto pop_back_4 = [&]() -> expected<void, error> {
    return darray_obj.pop_back()
        .and_then([]([[maybe_unused]] MoveOnlyObject element)
                      -> expected<void, error> {
          ADD_FAILURE_AT(__FILE__, __LINE__);
          return {};
        })
        .or_else([&](error err) -> expected<void, error> {
          EXPECT_EQ("No elements to pop", err.message());
          return {};
        });
  };

  auto _ = push_back_1()
               .and_then(push_back_2)
               .and_then(push_back_3)
               .and_then(pop_back_1)
               .and_then(pop_back_2)
               .and_then(pop_back_3)
               .and_then(pop_back_4)
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

TEST(darray, pushPopBackConstCopy) {

  darray<CopyOnlyObject> darray_obj =
      darray<CopyOnlyObject>::builder{}.capacity(2).build();

  auto push_back_1 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"}};
    CopyOnlyObject element{"1"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        1,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto push_back_2 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"},
                                        CopyOnlyObject{"2"}};
    CopyOnlyObject element{"2"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        2,
        2,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };
  auto push_back_3 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"}};
    CopyOnlyObject element{"3"};
    TestSpecPushBack<CopyOnlyObject, CopyOnlyObject &> spec = {
        darray_obj,
        {},
        element,
        3,
        4,
        span{expected_values,
             sizeof(expected_values) / sizeof(CopyOnlyObject)}};
    return tester_push_back(&spec).and_then(
        test_spec_push_back_to_void<CopyOnlyObject, CopyOnlyObject &>);
  };

  auto pop_back_1 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"},
                                        CopyOnlyObject{"2"}};
    return tester_pop_back(darray_obj, CopyOnlyObject{"3"}, 2, 2,
                           span{expected_values, sizeof(expected_values) /
                                                     sizeof(CopyOnlyObject)});
  };
  auto pop_back_2 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"}};
    return tester_pop_back(darray_obj, CopyOnlyObject{"2"}, 1, 2,
                           span{expected_values, sizeof(expected_values) /
                                                     sizeof(CopyOnlyObject)});
  };
  auto pop_back_3 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[1]; // "1" to silence compiler
    return tester_pop_back(darray_obj, CopyOnlyObject{"1"}, 0, 2,
                           span{expected_values, 0});
  };
  auto pop_back_4 = [&]() -> expected<void, error> {
    return darray_obj.pop_back()
        .and_then([]([[maybe_unused]] CopyOnlyObject &&element)
                      -> expected<void, error> {
          ADD_FAILURE_AT(__FILE__, __LINE__);
          return {};
        })
        .or_else([&](error err) -> expected<void, error> {
          EXPECT_EQ("No elements to pop", err.message());
          return {};
        });
  };

  auto _ = push_back_1()
               .and_then(push_back_2)
               .and_then(push_back_3)
               .and_then(pop_back_1)
               .and_then(pop_back_2)
               .and_then(pop_back_3)
               .and_then(pop_back_4)
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

TEST(darray, extractClearConstCopy) {

  darray<CopyOnlyObject> darray_obj =
      darray<CopyOnlyObject>::builder{}.capacity(2).build();

  auto insert_at_0 = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"2"}};
    CopyOnlyObject element{"2"};
    return tester_insert(0, darray_obj, element, 1, 2,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };
  auto insert_at_zero_again = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {CopyOnlyObject{"1"},
                                        CopyOnlyObject{"2"}};
    CopyOnlyObject element{"1"};
    return tester_insert(0, darray_obj, element, 2, 2,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };
  auto insert_at_last = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"}};
    CopyOnlyObject element{"3"};
    return tester_insert(darray_obj.pod().size(), darray_obj, element, 3, 4,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };
  auto insert_at_last_again = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"},
        CopyOnlyObject{"6"}};
    CopyOnlyObject element{"6"};
    return tester_insert(darray_obj.pod().size(), darray_obj, element, 4, 4,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };
  auto insert_at_second_last = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"},
        CopyOnlyObject{"5"}, CopyOnlyObject{"6"}};
    CopyOnlyObject element{"5"};
    return tester_insert(darray_obj.pod().size() - 1, darray_obj, element, 5, 8,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };
  auto insert_at_third_last = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"1"}, CopyOnlyObject{"2"}, CopyOnlyObject{"3"},
        CopyOnlyObject{"4"}, CopyOnlyObject{"5"}, CopyOnlyObject{"6"}};
    CopyOnlyObject element{"4"};
    return tester_insert(darray_obj.pod().size() - 2, darray_obj, element, 6, 8,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(CopyOnlyObject)});
  };

  auto extract_first_element = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"2"}, CopyOnlyObject{"3"}, CopyOnlyObject{"4"},
        CopyOnlyObject{"5"}, CopyOnlyObject{"6"}};
    return tester_extract(0, darray_obj, CopyOnlyObject{"1"}, 5, 8,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(CopyOnlyObject)});
  };
  auto extract_last_element = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"2"}, CopyOnlyObject{"3"}, CopyOnlyObject{"4"},
        CopyOnlyObject{"5"}};
    return tester_extract(darray_obj.pod().size() - 1, darray_obj,
                          CopyOnlyObject{"6"}, 4, 4,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(CopyOnlyObject)});
  };
  auto extract_second_element = [&]() -> expected<void, error> {
    CopyOnlyObject expected_values[] = {
        CopyOnlyObject{"2"}, CopyOnlyObject{"4"}, CopyOnlyObject{"5"}};
    return tester_extract(1, darray_obj, CopyOnlyObject{"3"}, 3, 4,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(CopyOnlyObject)});
  };
  auto clear = [&]() -> expected<void, error> {
    return darray_obj.clear()
        .and_then([&]() -> expected<void, error> {
          return darray_obj.clear().and_then([&]() -> expected<void, error> {
            EXPECT_EQ((size_t)0, darray_obj.pod().size());
            EXPECT_EQ((size_t)2, darray_obj.pod().capacity());
            return {};
          });
        })
        .or_else([](error err) -> expected<void, error> {
          ADD_FAILURE_AT(__FILE__, __LINE__);
          return unexpected{err};
        });
  };

  auto _ = insert_at_0()
               .and_then(insert_at_zero_again)
               .and_then(insert_at_last)
               .and_then(insert_at_last_again)
               .and_then(insert_at_second_last)
               .and_then(insert_at_third_last)
               .and_then(extract_first_element)
               .and_then(extract_last_element)
               .and_then(extract_second_element)
               .and_then(clear)
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

TEST(darray, extractClearMove) {

  darray<MoveOnlyObject> darray_obj =
      darray<MoveOnlyObject>::builder{}.capacity(2).build();

  auto insert_at_0 = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"2"}};
    return tester_insert(0, darray_obj, MoveOnlyObject{"2"}, 1, 2,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };
  auto insert_at_zero_again = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {MoveOnlyObject{"1"},
                                        MoveOnlyObject{"2"}};
    return tester_insert(0, darray_obj, MoveOnlyObject{"1"}, 2, 2,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };
  auto insert_at_last = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"1"}, MoveOnlyObject{"2"}, MoveOnlyObject{"3"}};
    return tester_insert(darray_obj.pod().size(), darray_obj,
                         MoveOnlyObject{"3"}, 3, 4,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };
  auto insert_at_last_again = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"1"}, MoveOnlyObject{"2"}, MoveOnlyObject{"3"},
        MoveOnlyObject{"6"}};
    return tester_insert(darray_obj.pod().size(), darray_obj,
                         MoveOnlyObject{"6"}, 4, 4,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };
  auto insert_at_second_last = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"1"}, MoveOnlyObject{"2"}, MoveOnlyObject{"3"},
        MoveOnlyObject{"5"}, MoveOnlyObject{"6"}};
    return tester_insert(darray_obj.pod().size() - 1, darray_obj,
                         MoveOnlyObject{"5"}, 5, 8,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };
  auto insert_at_third_last = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"1"}, MoveOnlyObject{"2"}, MoveOnlyObject{"3"},
        MoveOnlyObject{"4"}, MoveOnlyObject{"5"}, MoveOnlyObject{"6"}};
    return tester_insert(darray_obj.pod().size() - 2, darray_obj,
                         MoveOnlyObject{"4"}, 6, 8,
                         span{expected_values, sizeof(expected_values) /
                                                   sizeof(MoveOnlyObject)});
  };

  auto extract_first_element = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"2"}, MoveOnlyObject{"3"}, MoveOnlyObject{"4"},
        MoveOnlyObject{"5"}, MoveOnlyObject{"6"}};
    return tester_extract(0, darray_obj, MoveOnlyObject{"1"}, 5, 8,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(MoveOnlyObject)});
  };
  auto extract_last_element = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"2"}, MoveOnlyObject{"3"}, MoveOnlyObject{"4"},
        MoveOnlyObject{"5"}};
    return tester_extract(darray_obj.pod().size() - 1, darray_obj,
                          MoveOnlyObject{"6"}, 4, 4,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(MoveOnlyObject)});
  };
  auto extract_second_element = [&]() -> expected<void, error> {
    MoveOnlyObject expected_values[] = {
        MoveOnlyObject{"2"}, MoveOnlyObject{"4"}, MoveOnlyObject{"5"}};
    return tester_extract(1, darray_obj, MoveOnlyObject{"3"}, 3, 4,
                          span{expected_values, sizeof(expected_values) /
                                                    sizeof(MoveOnlyObject)});
  };
  auto clear = [&]() -> expected<void, error> {
    return darray_obj.clear()
        .and_then([&]() -> expected<void, error> {
          return darray_obj.clear().and_then([&]() -> expected<void, error> {
            EXPECT_EQ((size_t)0, darray_obj.pod().size());
            EXPECT_EQ((size_t)2, darray_obj.pod().capacity());
            return {};
          });
        })
        .or_else([](error err) -> expected<void, error> {
          ADD_FAILURE_AT(__FILE__, __LINE__);
          return unexpected{err};
        });
  };

  auto _ = insert_at_0()
               .and_then(insert_at_zero_again)
               .and_then(insert_at_last)
               .and_then(insert_at_last_again)
               .and_then(insert_at_second_last)
               .and_then(insert_at_third_last)
               .and_then(extract_first_element)
               .and_then(extract_last_element)
               .and_then(extract_second_element)
               .and_then(clear)
               .or_else([](error err) -> expected<void, error> {
                 ADD_FAILURE_AT(__FILE__, __LINE__);
                 return unexpected{err};
               });
}

static_assert(std::contiguous_iterator<darray<int>::iterator>);
static_assert(std::contiguous_iterator<darray<CopyOnlyObject>::iterator>);
static_assert(std::contiguous_iterator<darray<MoveOnlyObject>::iterator>);
TEST(darray, iterator) {
  unsigned int test_data[] = {10, 4, 6, 8, 3, 11, 5, 9, 1};

  darray<unsigned int> darray_obj =
      darray<unsigned int>::builder{}.capacity(2).build();
  for (auto element :
       span{test_data, sizeof(test_data) / sizeof(unsigned int)}) {
    EXPECT_TRUE(darray_obj.push_back(element).has_value());
  }

  // use with range-based for loop
  size_t idx = 0;
  for (auto element : darray_obj) {
    EXPECT_EQ(test_data[idx++], element);
  }

  // use with old style iterator loop
  idx = 0;
  for (auto b = darray_obj.begin(); b != darray_obj.end(); b++) {
    EXPECT_EQ(test_data[idx++], *b);
  }

  // algorithms compatibility
  unsigned int test_data_sorted[] = {1, 3, 4, 5, 6, 8, 9, 10, 11};
  sort(darray_obj);
  idx = 0;
  for (auto element : darray_obj) {
    EXPECT_EQ(test_data_sorted[idx++], element);
  }

  // back_inserter (LegacyOutputIterator) compatibility
  unsigned int test_data_transformed[] = {2, 6, 8, 10, 12, 16, 18, 20, 22};
  darray<unsigned int> transformed{};
  transform(darray_obj, std::back_inserter(transformed),
            [](unsigned int &element) { return element * 2; });
  idx = 0;
  for (auto element : transformed) {
    EXPECT_EQ(test_data_transformed[idx++], element);
  }
}
