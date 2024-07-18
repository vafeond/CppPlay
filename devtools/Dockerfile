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

ARG IMAGE_NAME

FROM $IMAGE_NAME as base

ARG DEBIAN_FRONTEND=noninteractive

ENV LANG=C.UTF-8

RUN apt-get update
RUN apt-get install -y unzip
RUN apt-get install -y curl

#==========================
# neovim
RUN wget https://github.com/neovim/neovim/releases/download/v0.9.5/nvim-linux64.tar.gz
RUN mkdir /home/nvim
RUN mv nvim-linux64.tar.gz /home/nvim
RUN cd /home/nvim; tar -xf nvim-linux64.tar.gz
RUN ln -s /home/nvim/nvim-linux64/bin/nvim /usr/bin/nvim

