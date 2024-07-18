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

#include <algorithm>
#include <cstddef> // size_t & ptrdiff_t
#include <cstring>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <new>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace CppPlay {

using std::bad_alloc;
using std::expected;
using std::format;
using std::forward;
using std::function;
using std::is_move_assignable_v;
using std::is_trivially_copyable_v;
using std::lock_guard;
using std::make_unique;
using std::memcpy;
using std::move;
using std::optional;
using std::scoped_lock;
using std::span;
using std::string;
using std::unexpected;
using std::unique_ptr;
using std::ranges::for_each;
using std::ranges::views::reverse;

using std::mutex;

// custom error object
struct error {
  string m_message;
  explicit error(string msg) : m_message{msg} {};
  [[nodiscard]] string message() const noexcept { return m_message; }
};

//
// type traits for behavior specification
//

// use as ThreadProtection template parameter
// to disable thread protection, enabling best performance
template <typename T> struct ThreadProtectionDisabled {
  static constexpr bool do_multithreaded_protection = false;
};

// use as ThreadProtection template parameter
// to enable thread protection
template <typename T> struct ThreadProtectionEnabled {
  static constexpr bool do_multithreaded_protection = true;
};

template <typename T, typename ThreadProtection = ThreadProtectionDisabled<T>>
class darray {
  constinit static const size_t DEFAULT_RESERVE_SIZE = 8;
  unique_ptr<T[]> m_buffer;
  size_t m_capacity;
  size_t m_original_capacity;
  size_t m_size;

  // no conditional member variable support as yet, so all this foo is to
  // reduce impact of m_mutex member when thread protection disabled
  struct Empty {};
  using ConditionalMutex =
      std::conditional<ThreadProtection::do_multithreaded_protection,
                       std::mutex, Empty>::type;
  [[no_unique_address]] mutable ConditionalMutex m_mutex;

  // construct darray specifying initial capacity
  constexpr explicit darray(std::size_t initial_capacity)
      : m_buffer{make_unique<T[]>(initial_capacity)},
        m_capacity{initial_capacity}, m_original_capacity{initial_capacity},
        m_size{0} {}

  struct ProcessingData {
    explicit ProcessingData(size_t current_capacity)
        : buffer_resized{}, buffer_resized_capacity{current_capacity} {}
    optional<unique_ptr<T[]>> buffer_resized;
    size_t buffer_resized_capacity;
  };

  constexpr inline auto buffer_create(ProcessingData *const p_data,
                                      const size_t capacity) noexcept
      -> expected<ProcessingData *const, error> {
    try {
      unique_ptr<T[]> buffer_resized = make_unique<T[]>(capacity);
      p_data->buffer_resized = optional{move(buffer_resized)};
      p_data->buffer_resized_capacity = capacity;
      return {p_data};
    } catch (bad_alloc &err) {
      return unexpected{error{format("{}", err.what())}};
    }
  }

  constexpr inline auto
  buffer_increase_if(ProcessingData *const p_data,
                     const function<bool(void)> &predicate) noexcept
      -> expected<ProcessingData *const, error> {
    [[likely]] if (false == predicate()) { return {p_data}; }
    return buffer_create(p_data, m_capacity << 1);
  }
  constexpr inline auto
  buffer_decrease_if(ProcessingData *const p_data,
                     const function<bool(void)> &predicate) noexcept
      -> expected<ProcessingData *const, error> {
    [[likely]] if (false == predicate()) { return {p_data}; }
    return buffer_create(p_data, m_capacity >> 1);
  }
  constexpr inline auto buffer_reset_original_capacity_if(
      ProcessingData *const p_data,
      const function<bool(void)> &predicate) noexcept
      -> expected<ProcessingData *const, error> {
    [[likely]] if (false == predicate()) { return {p_data}; }
    return buffer_create(p_data, m_original_capacity);
  }

  template <typename U>
  inline auto emplace(U &&element, size_t idx) noexcept
      -> expected<void, error> {
    m_buffer[idx] = forward<U>(element);
    return {};
  }

  // only valid for copyable objects
  // used when objects should be copyied, such as copy constructor and copy
  // assignment
  inline auto buffer_copy(const span<T> source, span<T> dest) noexcept
      -> expected<size_t, error> {
    for_each(source,
             [&dest, idx = 0](T &value) mutable { dest[idx++] = value; });
    return {dest.size()};
  }

  enum TransferMethod { DIRECT, SHIFT_LEFT, SHIFT_RIGHT };
  struct TransferMethodDirect {
    static constexpr TransferMethod method = DIRECT;
  };
  struct TransferMethodLeft {
    static constexpr TransferMethod method = SHIFT_LEFT;
  };
  struct TransferMethodRight {
    static constexpr TransferMethod method = SHIFT_RIGHT;
  };
  // valid for moveable and "copy only" objects, preferring move
  // used when objects should be transfered between buffers, such as resize,
  // move constructor and move assignment
  template <typename Method>
  inline auto buffer_transfer(const span<T> source, span<T> dest) noexcept
      -> void {
    // could memcpy be used for is_trivially_copyable DIRECT, or memove for
    // SHIFT_LEFT and SHIFT_RIGHT? (memove would be great as it correctly uses
    // forward or reverse copying depending on buffer overlap)
    // memcopy  and memmove can only be used if "not potentially-overlapping
    // subobjects" which AFAIK can't be detected
    //  - https://en.cppreference.com/w/cpp/types/is_trivially_copyable
    //  - https://en.cppreference.com/w/cpp/string/byte/memmove
    if constexpr (SHIFT_RIGHT == Method::method) {
      for_each(source | reverse,
               [&dest, idx = source.size()](T &value) mutable {
                 if constexpr (is_move_assignable_v<T>) {
                   dest[--idx] = move(value);
                 } else {
                   dest[--idx] = value;
                 }
               });
    } else {
      for_each(source, [&dest, idx = 0](T &value) mutable {
        if constexpr (is_move_assignable_v<T>) {
          dest[idx++] = move(value);
        } else {
          dest[idx++] = value;
        }
      });
    }
  }

  constexpr auto
  transfer_direct_if_buffer_resized(ProcessingData *const p_data) noexcept
      -> expected<ProcessingData *const, error> {
    [[likely]] if (false == p_data->buffer_resized.has_value()) {
      return {p_data};
    }
    buffer_transfer<TransferMethodDirect>(
        span{m_buffer.get(), m_size},
        span{p_data->buffer_resized.value().get(), m_size});
    return {p_data};
  };

  constexpr auto transfer_erase(ProcessingData *const p_data,
                                const size_t erase_index) noexcept
      -> expected<ProcessingData *const, error> {
    const span<T> src_left{m_buffer.get(), erase_index};
    const span<T> src_right{&(m_buffer.get()[erase_index + 1]),
                            m_size - (erase_index + 1)};
    span<T> dest_right{&(m_buffer.get()[erase_index]), src_right.size()};

    std::function transfer_left = [&]() -> expected<void, error> { return {}; };

    if (p_data->buffer_resized.has_value()) {
      transfer_left = [&]() -> expected<void, error> {
        if (0 == src_left.size()) {
          return {};
        }
        const span<T> dest_left{p_data->buffer_resized.value().get(),
                                src_left.size()};
        buffer_transfer<TransferMethodDirect>(src_left, dest_left);
        return {};
      };

      dest_right = span{&(p_data->buffer_resized.value().get()[erase_index]),
                        src_right.size()};
    }

    const auto transfer_right = [&]() -> expected<void, error> {
      if (0 == src_right.size()) {
        return {};
      }
      buffer_transfer<TransferMethodLeft>(src_right, dest_right);
      return {};
    };

    return transfer_left()
        .and_then(transfer_right)
        .and_then([p_data]() -> expected<ProcessingData *const, error> {
          return {p_data};
        });
  };

  constexpr auto transfer_insert(ProcessingData *const p_data,
                                 const size_t insert_index) noexcept
      -> expected<ProcessingData *const, error> {
    const span<T> src_left{m_buffer.get(), insert_index};
    const span<T> src_right{&(m_buffer.get()[insert_index]),
                            m_size - insert_index};
    span<T> dest_right{&(m_buffer.get()[insert_index + 1]), src_right.size()};

    std::function transfer_left = [&]() -> expected<void, error> { return {}; };

    if (p_data->buffer_resized.has_value()) {
      transfer_left = [&]() -> expected<void, error> {
        if (0 == src_left.size()) {
          return {};
        }
        span<T> dest_left{p_data->buffer_resized.value().get(),
                          src_left.size()};
        buffer_transfer<TransferMethodDirect>(src_left, dest_left);
        return {};
      };

      dest_right =
          span{&(p_data->buffer_resized.value().get()[insert_index + 1]),
               src_right.size()};
    }

    const auto transfer_right = [&]() -> expected<void, error> {
      if (0 == src_right.size()) {
        return {};
      }
      buffer_transfer<TransferMethodRight>(src_right, dest_right);
      return {};
    };

    return transfer_left()
        .and_then(transfer_right)
        .and_then([p_data]() -> expected<ProcessingData *const, error> {
          return {p_data};
        });
  };

  // darray helpers
  inline auto darray_copy_into_me(const darray &other) noexcept
      -> expected<size_t, error> {

    const auto copy =
        [&](ProcessingData *p_data) -> expected<ProcessingData *const, error> {
      return buffer_copy(
                 span{other.m_buffer.get(), other.m_size},
                 span{p_data->buffer_resized.value().get(), other.m_size})
          .and_then([p_data]([[maybe_unused]] size_t size)
                        -> expected<ProcessingData *const, error> {
            return {p_data};
          });
    };

    const auto process = [&]() {
      ProcessingData data{m_capacity};
      return buffer_create(&data, other.m_capacity)
          .and_then(copy)
          .and_then([&](ProcessingData *const p_data) {
            m_buffer.swap(p_data->buffer_resized.value());
            m_capacity = other.m_capacity;
            m_original_capacity = other.m_original_capacity;
            m_size = other.m_size;
            return expected<size_t, error>{m_size};
          });
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const scoped_lock<mutex> lock(m_mutex, other.m_mutex);
      return process();
    } else {
      return process();
    }
  }

  inline auto darray_move_into_me(darray &&other) noexcept
      -> expected<size_t, error> {
    const auto process = [&]() -> expected<size_t, error> {
      m_buffer = move(other.m_buffer);
      m_capacity = other.m_capacity;
      m_original_capacity = other.m_original_capacity;
      m_size = other.m_size;
      return {m_size};
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const scoped_lock<mutex> lock(m_mutex, other.m_mutex);
      return process();
    } else {
      return process();
    }
  }

public:
  //
  // special member functions
  //

  constexpr darray()
      : m_buffer{make_unique<T[]>(DEFAULT_RESERVE_SIZE)},
        m_capacity{DEFAULT_RESERVE_SIZE},
        m_original_capacity{DEFAULT_RESERVE_SIZE}, m_size{0} {}

  constexpr ~darray() = default;

  constexpr darray(const darray &other) {
    // thread protection provided in darray_copy_into_me
    darray_copy_into_me(other).or_else(
        [](error err) -> expected<size_t, error> { throw err; });
  }

  constexpr auto operator=(const darray &other) -> darray & {
    // thread protection provided in darray_copy_into_me
    darray_copy_into_me(other).or_else([](error err) { throw err; });
    return this;
  }

  constexpr explicit darray(T &&other) noexcept {
    // thread protection provided in darray_move_into_me
    darray_move_into_me(other).or_else(
        [](error err) -> expected<size_t, error> { throw err; });
  };

  constexpr auto operator=(T &&other) noexcept -> T & {
    // thread protection provided in darray_move_into_me
    darray_move_into_me(other).or_else(
        [](error err) -> expected<size_t, error> { throw err; });
  };

  //
  // darray builder helper
  //
  class builder {
    size_t m_initial_capacity = DEFAULT_RESERVE_SIZE;

  public:
    constexpr auto capacity(size_t capacity) noexcept -> builder & {
      m_initial_capacity = capacity;
      return *this;
    };
    [[nodiscard]] constexpr auto build() const noexcept -> darray {
      return darray{m_initial_capacity};
    };
  };
  friend builder;

  // user modification of capacity outside of construction is not allowed
  // this prevents the user accidentally changing push_back and pop_back
  // complextiy from amortized constant to quadratic
  // auto reserve() noexcept -> std::expected<std::size_t,error>;

  //
  // store
  //
  template <typename U>
  auto push_back(U &&new_element) noexcept -> expected<size_t, error> {
    const auto resize_if_at_capacity = [&](ProcessingData *const p_data) {
      return buffer_increase_if(p_data, [&]() { return m_size == m_capacity; });
    };

    const auto transfer_if_resized = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      return transfer_direct_if_buffer_resized(p_data);
    };

    const auto use_new_buffer_if_resized = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      if (p_data->buffer_resized.has_value()) {
        m_buffer.swap(p_data->buffer_resized.value());
        m_capacity = p_data->buffer_resized_capacity;
      }
      return {p_data};
    };

    const auto store_new_element =
        [&]([[maybe_unused]] ProcessingData *const p_data)
        -> expected<size_t, error> {
      return emplace(forward<U>(new_element), m_size)
          .and_then([&]() -> expected<size_t, error> { return {++m_size}; });
    };

    const auto process = [&]() {
      ProcessingData data{m_capacity};
      return resize_if_at_capacity(&data)
          .and_then(transfer_if_resized)
          .and_then(use_new_buffer_if_resized)
          .and_then(store_new_element);
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  // alternate implementation without monadic error handling and the composition
  // it enables
  // used to benchmark to help understand tradeoffs of both approaches
  template <typename U>
  auto push_back_2(U &&new_element) noexcept -> expected<size_t, error> {

    const auto process = [&]() -> expected<size_t, error> {
      if (m_size == m_capacity) {
        try {
          size_t buffer_resized_capacity = m_capacity << 1;
          unique_ptr<T[]> buffer_resized =
              make_unique<T[]>(buffer_resized_capacity);

          auto src = m_buffer.get();
          auto dest = buffer_resized.get();
          for (size_t idx = 0; idx < m_size; idx++) {
            if constexpr (is_move_assignable_v<T>) {
              dest[idx] = move(src[idx]);
            } else {
              dest[idx] = src[idx];
            }
          }

          m_buffer.swap(buffer_resized);
          m_capacity = buffer_resized_capacity;
        } catch (bad_alloc &err) {
          return unexpected{error{format("{}", err.what())}};
        }
      }
      m_buffer[m_size++] = forward<U>(new_element);
      return {m_size};
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  template <typename U>
  auto insert(U &&new_element, const size_t index) noexcept
      -> expected<size_t, error> {

    const auto check_preconditions = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      if (index > m_size) {
        return unexpected{
            error{format("Invalid insert criteria. Index ({}) beyond length of "
                         "contiguous elements ({})",
                         index, m_size)}};
      }
      return {p_data};
    };

    const auto resize_if_at_capacity = [&](ProcessingData *const p_data) {
      return buffer_increase_if(p_data, [&]() { return m_size == m_capacity; });
    };

    const auto transfer_contents_as_needed = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      return transfer_insert(p_data, index);
    };

    const auto use_new_buffer_if_resized = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      if (p_data->buffer_resized.has_value()) {
        m_buffer.swap(p_data->buffer_resized.value());
        m_capacity = p_data->buffer_resized_capacity;
      }
      return {p_data};
    };

    const auto store_new_element =
        [&]([[maybe_unused]] ProcessingData *const p_data)
        -> expected<size_t, error> {
      return emplace(forward<U>(new_element), index)
          .and_then([&]() -> expected<size_t, error> { return {++m_size}; });
    };

    const auto process = [&]() {
      ProcessingData data{m_capacity};
      return check_preconditions(&data)
          .and_then(resize_if_at_capacity)
          .and_then(transfer_contents_as_needed)
          .and_then(use_new_buffer_if_resized)
          .and_then(store_new_element);
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  //
  // delete
  //
  auto pop_back() noexcept -> expected<T, error> {

    const auto resize_if_size_half_capacity =
        [&](ProcessingData *const p_data) {
          return buffer_decrease_if(p_data,
                                    [&]() {
                                      return (
                                          (m_capacity > m_original_capacity) &&
                                          (m_size <= (m_capacity >> 1)));
                                    })
              // ignore buffer resize error, failure to allocate different
              // buffer does not invalidate any class invariants
              .or_else([p_data]([[maybe_unused]] error err)
                           -> expected<ProcessingData *const, error> {
                return {p_data};
              });
        };

    const auto use_new_buffer_if_resized = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      if (p_data->buffer_resized.has_value()) {
        m_buffer.swap(p_data->buffer_resized.value());
        m_capacity = p_data->buffer_resized_capacity;
      }
      return {p_data};
    };

    auto process = [&]() -> expected<T, error> {
      [[unlikely]] if (0 == m_size) {
        return unexpected{error{format("No elements to pop")}};
      }

      // pull element directly into return type so can be used for RVO, minimize
      // copy/move
      expected<T, error> ret;
      if constexpr (is_move_assignable_v<T>) {
        ret = {std::move(m_buffer[--m_size])};
      } else {
        ret = {m_buffer[--m_size]};
      }

      const auto transfer_contents_as_needed = [&](ProcessingData *const p_data)
          -> expected<ProcessingData *const, error> {
        return transfer_direct_if_buffer_resized(p_data).or_else(
            [&](error err) -> expected<ProcessingData *const, error> {
              // transfer error? smaller buffer not used, so can safely place
              // element back to avoid data loss
              if constexpr (is_move_assignable_v<T>) {
                m_buffer[m_size++] = std::move(ret.value());
              } else {
                m_buffer[m_size++] = ret.value();
              }
              return unexpected{err};
            });
      };

      ProcessingData data{m_capacity};
      expected<ProcessingData *const, error> proc_result =
          resize_if_size_half_capacity(&data)
              .and_then(transfer_contents_as_needed)
              .and_then(use_new_buffer_if_resized);

      if (proc_result.has_value()) {
        return ret;
      }
      return unexpected{proc_result.error()};
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  auto extract(const std::size_t index) noexcept -> std::expected<T, error> {

    auto process = [&]() -> expected<T, error> {
      [[unlikely]] if (index >= m_size) {
        return unexpected{error{format("Cannot extract, index too large")}};
      }

      // pull element directly into return type so can be used for RVO, minimize
      // copy/move
      expected<T, error> ret;
      if constexpr (is_move_assignable_v<T>) {
        ret = {std::move(m_buffer[index])};
      } else {
        ret = {m_buffer[index]};
      }

      const auto resize_if_size_half_capacity =
          [&](ProcessingData *const p_data) {
            return buffer_decrease_if(
                       p_data,
                       [&]() {
                         return ((m_capacity > m_original_capacity) &&
                                 ((m_size - 1) <= (m_capacity >> 1)));
                       })
                // ignore buffer resize error, failure to allocate different
                // buffer does not invalidate any class invariants
                .or_else([p_data]([[maybe_unused]] error err)
                             -> expected<ProcessingData *const, error> {
                  return {p_data};
                });
          };

      const auto transfer_contents_as_needed = [&](ProcessingData *const p_data)
          -> expected<ProcessingData *const, error> {
        return transfer_erase(p_data, index)
            .or_else([&](error err) -> expected<ProcessingData *const, error> {
              // transfer error? smaller buffer not used, so can safely place
              // element back to avoid data loss
              if constexpr (is_move_assignable_v<T>) {
                m_buffer[index] = std::move(ret.value());
              } else {
                m_buffer[index] = ret.value();
              }
              return unexpected{err};
            });
      };

      const auto use_new_buffer_if_resized = [&](ProcessingData *const p_data)
          -> expected<ProcessingData *const, error> {
        if (p_data->buffer_resized.has_value()) {
          m_buffer.swap(p_data->buffer_resized.value());
          m_capacity = p_data->buffer_resized_capacity;
        }
        return {p_data};
      };

      ProcessingData data{m_capacity};
      expected<ProcessingData *const, error> proc_result =
          resize_if_size_half_capacity(&data)
              .and_then(transfer_contents_as_needed)
              .and_then(use_new_buffer_if_resized)
              .and_then([&](ProcessingData *const p_data)
                            -> expected<ProcessingData *const, error> {
                m_size--;
                return {p_data};
              });

      if (proc_result.has_value()) {
        return ret;
      }
      return unexpected{proc_result.error()};
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  auto clear() noexcept -> std::expected<void, error> {

    const auto resize_if_not_original_size = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      return buffer_reset_original_capacity_if(
                 p_data, [&]() { return m_capacity > m_original_capacity; })
          .and_then([&](ProcessingData *const p_data)
                        -> expected<ProcessingData *const, error> {
            if (false == p_data->buffer_resized.has_value()) {
              return {p_data};
            }
            m_buffer.swap(p_data->buffer_resized.value());
            m_capacity = p_data->buffer_resized_capacity;
            m_size = 0;
            return {p_data};
          });
    };

    const auto clear_elements_if_not_resized = [&](ProcessingData *const p_data)
        -> expected<ProcessingData *const, error> {
      for_each(span{m_buffer.get(), m_size},
               [&, idx = 0]([[maybe_unused]] T &element) mutable {
                 if constexpr (is_move_assignable_v<T>) {
                   m_buffer[idx++] = T{};
                 } else {
                   T default_element{};
                   m_buffer[idx++] = default_element;
                 }
               });
      m_size = 0;
      return {p_data};
    };

    const auto process = [&]() {
      ProcessingData data{m_capacity};
      return resize_if_not_original_size(&data)
          .and_then(clear_elements_if_not_resized)
          .and_then([]([[maybe_unused]] ProcessingData *const p_data)
                        -> expected<void, error> { return {}; });
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }

  //
  // access - iterator (random access)
  //
  using value_type =
      T; // needed for compatability with "back_inserter" (LegacyOutputIterator)
  struct iterator {
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using element_type = T;
    using pointer = element_type *;
    using reference = element_type &;

    iterator() = default;
    explicit iterator(pointer p) { m_ptr = p; }

    reference operator*() const { return *m_ptr; }
    pointer operator->() const { return m_ptr; }

    iterator &operator++() {
      m_ptr++;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    iterator &operator+=(int i) {
      m_ptr += i;
      return *this;
    }
    iterator operator+(const difference_type other) const {
      return iterator{m_ptr + other};
    }
    friend iterator operator+(const difference_type value,
                              const iterator &other) {
      return iterator{other + value};
    }

    iterator &operator--() {
      m_ptr--;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }
    iterator &operator-=(int i) {
      m_ptr -= i;
      return *this;
    }
    difference_type operator-(const iterator &other) const {
      return m_ptr - other.m_ptr;
    }
    iterator operator-(const difference_type other) const {
      return iterator{m_ptr - other};
    }
    friend iterator operator-(const difference_type value,
                              const iterator &other) {
      return iterator{other - value};
    }

    reference operator[](difference_type idx) const { return m_ptr[idx]; }

    auto operator<=>(const iterator &) const = default;

  private:
    pointer m_ptr;
  };
  auto begin() const noexcept -> iterator { return iterator{m_buffer.get()}; }
  auto end() const noexcept -> iterator {
    return iterator{&(m_buffer.get()[m_size])};
  }

  //
  // access - random
  //
  [[nodiscard]] auto at(std::size_t idx) const noexcept
      -> std::expected<iterator, error> {

    const auto process = [&]() -> std::expected<iterator, error> {
      [[unlikely]] if (idx >= m_size) {
        return unexpected{error{format(
            "Requested index {} beyond array end (size={})", idx, m_size)}};
      }
      return {iterator{&(m_buffer[idx])}};
    };

    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return process();
    } else {
      return process();
    }
  }
  auto operator[](std::size_t idx) const -> T & {
    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return m_buffer[idx];
    } else {
      return m_buffer[idx];
    }
  }

  //
  // metadata
  //

  [[nodiscard]] auto capacity() const noexcept -> expected<std::size_t, error> {
    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return {m_capacity};
    } else {
      return {m_capacity};
    }
  }

  [[nodiscard]] auto size() const noexcept
      -> std::expected<std::size_t, error> {
    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return {m_size};
    } else {
      return {m_size};
    }
  }

  [[nodiscard]] auto is_empty() const noexcept -> std::expected<bool, error> {
    if constexpr (ThreadProtection::do_multithreaded_protection) {
      const std::lock_guard<std::mutex> lock(m_mutex);
      return {(0 == m_size)};
    } else {
      return {(0 == m_size)};
    }
  }

  // non-monadic (plain-old-data return value) metadata accessors
  class pod_metadata_accessor {
    const darray &m_darray;
    constexpr explicit pod_metadata_accessor(const darray &darray_obj)
        : m_darray{darray_obj} {}
    friend darray;

  public:
    [[nodiscard]] constexpr auto capacity() const noexcept -> std::size_t {
      if constexpr (ThreadProtection::do_multithreaded_protection) {
        const std::lock_guard<std::mutex> lock(m_darray.m_mutex);
        return m_darray.m_capacity;
      } else {
        return m_darray.m_capacity;
      }
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
      if constexpr (ThreadProtection::do_multithreaded_protection) {
        const std::lock_guard<std::mutex> lock(m_darray.m_mutex);
        return m_darray.m_size;
      } else {
        return m_darray.m_size;
      }
    }

    [[nodiscard]] constexpr auto is_empty() const noexcept -> bool {
      if constexpr (ThreadProtection::do_multithreaded_protection) {
        const std::lock_guard<std::mutex> lock(m_darray.m_mutex);
        return (0 == m_darray.m_size);
      } else {
        return (0 == m_darray.m_size);
      }
    }
  };
  // get plain-old-data metadata accessor
  [[nodiscard]] constexpr auto pod() const noexcept
      -> const pod_metadata_accessor {
    return pod_metadata_accessor{*this};
  }
  friend pod_metadata_accessor;
};

} // namespace CppPlay
