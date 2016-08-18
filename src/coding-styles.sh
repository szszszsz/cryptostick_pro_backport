#!/bin/bash

#style is defined in .clang-format

# run for all files
#cat <(find . -name '*.cpp') <(find . -name '*.h') | xargs -n1 clang-format-3.8 -i

#run only on changed files vs master
find | grep -e '\.c$' -e '\.h$' | xargs -n1 clang-format-3.8 -i

