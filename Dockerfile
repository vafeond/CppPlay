#  This is free and unencumbered software released into the public domain.
#
#  Anyone is free to copy, modify, publish, use, compile, sell, or
#  distribute this software, either in source code form or as a compiled
#  binary, for any purpose, commercial or non-commercial, and by any
#  means.
#
#  In jurisdictions that recognize copyright laws, the author or authors
#  of this software dedicate any and all copyright interest in the
#  software to the public domain. We make this dedication for the benefit
#  of the public at large and to the detriment of our heirs and
#  successors. We intend this dedication to be an overt act of
#  relinquishment in perpetuity of all present and future rights to this
#  software under copyright law.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#  For more information, please refer to <https://unlicense.org>

FROM ubuntu:22.04

ARG GCC_VERSION=13
ARG CLANG_VERSION=17
ARG LLDB_VERSION=17

ARG GTEST_VERSION=1.15.0
ARG GBENCH_VERSION=1.8.5

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update
RUN apt-get install -y wget
RUN apt-get install -y lsb-release 
RUN apt-get install -y software-properties-common
RUN apt-get install -y gnupg
RUN apt-get install -y make
RUN apt-get install -y git

#==========================
# gcc
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get -y install gcc-$GCC_VERSION g++-$GCC_VERSION
RUN ln -s /usr/bin/g++-$GCC_VERSION /usr/bin/g++
RUN ln -s /usr/bin/gcc-$GCC_VERSION /usr/bin/gcc

#==========================
# clang & lldb
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod u+x llvm.sh
RUN ./llvm.sh $CLANG_VERSION
RUN apt-get install lldb-$LLDB_VERSION
RUN apt-get -y install clang-tidy-$LLDB_VERSION
RUN apt-get -y install clang-format-$LLDB_VERSION

#==========================
# static metrics tool
RUN wget https://github.com/metrixplusplus/metrixplusplus/archive/refs/tags/1.7.1.tar.gz
RUN mkdir /home/metrixplusplus
RUN mv 1.7.1.tar.gz /home/metrixplusplus
RUN cd /home/metrixplusplus; tar -xf 1.7.1.tar.gz

#==========================
# source dependencies
RUN mkdir /home/extra-sources

# gtest
RUN wget https://github.com/google/googletest/releases/download/v$GTEST_VERSION/googletest-$GTEST_VERSION.tar.gz
RUN mv googletest-$GTEST_VERSION.tar.gz /home/extra-sources
RUN cd /home/extra-sources; tar -xf googletest-$GTEST_VERSION.tar.gz

# gbench
RUN wget https://github.com/google/benchmark/archive/refs/tags/v$GBENCH_VERSION.tar.gz
RUN mv v$GBENCH_VERSION.tar.gz /home/extra-sources
RUN cd /home/extra-sources; tar -xf v$GBENCH_VERSION.tar.gz

# override build system EXTRA_SOURCES_ROOT to build from within container
ENV EXTRA_SOURCES_ROOT=/home/extra-sources


