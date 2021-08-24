#!/bin/bash

for file in $(git ls-files -- '*.cpp' '*.h'); do
    clang-format -i -style=file "$file"
done
