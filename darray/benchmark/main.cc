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

