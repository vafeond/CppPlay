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
