#include "darray.hpp"
#include "gtest.h"

#include <expected>
#include <future>

using CppPlay::darray;
using std::string;

//=============================================================================
// Helper Classes and Functions
//=============================================================================

//=============================================================================
// Tests
//=============================================================================

TEST(darrayProtected, test) {

  constexpr int ADD_LIMIT = 1000;

  darray<int, CppPlay::ThreadProtectionEnabled<int>> darray_obj =
      darray<int, CppPlay::ThreadProtectionEnabled<int>>::builder{}
          .capacity(2)
          .build();

  auto access_1 = [&]() {
    for (int idx = 0; idx < ADD_LIMIT; idx++) {
      EXPECT_TRUE(darray_obj.push_back(idx).has_value());
    }
  };

  auto f1 = std::async(access_1);
  auto f2 = std::async(access_1);
  auto f3 = std::async(access_1);

  f1.wait();
  f2.wait();
  f3.wait();

  EXPECT_EQ(ADD_LIMIT * 3, darray_obj.size());
}
