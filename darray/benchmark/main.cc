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

#include <benchmark/benchmark.h>
#include "darray.hpp"

#include <vector>
#include <chrono>

//
// push_back
//
static void BM_darray_push_back(benchmark::State& state) {
  CppPlay::darray<unsigned int> darray_obj = CppPlay::darray<unsigned int>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      darray_obj.push_back(idx); // ignore return value
    }
  }
}
BENCHMARK(BM_darray_push_back);

static void BM_darray_push_back_protected(benchmark::State& state) {
  CppPlay::darray<unsigned int,CppPlay::ThreadProtectionEnabled<unsigned int>> darray_obj = CppPlay::darray<unsigned int,CppPlay::ThreadProtectionEnabled<unsigned int>>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      darray_obj.push_back(idx); // ignore return value
    }
  }
}
BENCHMARK(BM_darray_push_back_protected);

static void BM_darray_push_back_2(benchmark::State& state) {
  CppPlay::darray<unsigned int> darray_obj = CppPlay::darray<unsigned int>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      darray_obj.push_back_2(idx); // ignore return value
    }
  }
}
BENCHMARK(BM_darray_push_back_2);

static void BM_darray_push_back_2_protected(benchmark::State& state) {
  CppPlay::darray<unsigned int,CppPlay::ThreadProtectionEnabled<unsigned int>> darray_obj = CppPlay::darray<unsigned int,CppPlay::ThreadProtectionEnabled<unsigned int>>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      darray_obj.push_back_2(idx); // ignore return value
    }
  }
}
BENCHMARK(BM_darray_push_back_2_protected);

static void BM_vec_push_back(benchmark::State& state) {
  std::vector<unsigned int> vec{};
  vec.reserve(8);
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      vec.push_back(idx);
    }
  }
}
BENCHMARK(BM_vec_push_back);


//
// insert_at_start
//
static void BM_darray_insert_at_start(benchmark::State& state) {
  CppPlay::darray<unsigned int> darray_obj = CppPlay::darray<unsigned int>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      auto start = std::chrono::high_resolution_clock::now();

      darray_obj.insert(idx,0); // ignore return value
      
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }
  }
}
BENCHMARK(BM_darray_insert_at_start);

static void BM_vec_insert_at_start(benchmark::State& state) {
  std::vector<unsigned int> vec{};
  vec.reserve(8);
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      auto insertIter=vec.cbegin();
      auto start = std::chrono::high_resolution_clock::now();

      vec.insert(insertIter,idx);
      
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }
  }
}
BENCHMARK(BM_vec_insert_at_start);


//
// insert_at_mid
//
static void BM_darray_insert_at_mid(benchmark::State& state) {
  CppPlay::darray<unsigned int> darray_obj = CppPlay::darray<unsigned int>::builder{}.capacity(8).build();
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      unsigned int insertIdx=darray_obj.pod().size()/2;
      auto start = std::chrono::high_resolution_clock::now();

      darray_obj.insert(idx,insertIdx); // ignore return value
      
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }
  }
}
BENCHMARK(BM_darray_insert_at_mid);

static void BM_vec_insert_at_mid(benchmark::State& state) {
  std::vector<unsigned int> vec{};
  vec.reserve(8);
  for ( auto _ : state ) {
    for ( unsigned int idx=0 ; idx<1000 ; idx++ ) {
      auto insertIter=vec.cbegin();
      for ( unsigned int x=0 ; x<vec.size()/2 ; x++, insertIter++ );
      auto start = std::chrono::high_resolution_clock::now();

      vec.insert(insertIter,idx);
      
      auto end = std::chrono::high_resolution_clock::now();
      auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);
      state.SetIterationTime(elapsed_seconds.count());
    }
  }
}
BENCHMARK(BM_vec_insert_at_mid);

