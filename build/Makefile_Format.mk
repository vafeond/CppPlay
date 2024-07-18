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

ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included after Makefile_Config.mk)
endif

format:
	$(info )
	$(info [ Format ] =============================================)
	$(info SOURCES                    : $(SOURCES))
	$(info FORMAT_EXTRA_FILES_RELATIVE: $(FORMAT_EXTRA_FILES_RELATIVE))
	$(info )
	$(FORMATTER) -i --style=llvm $(abspath $(FORMAT_EXTRA_FILES_RELATIVE)) $(abspath $(SOURCES))

.PHONY: format
