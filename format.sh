#!/bin/bash

files=$(git ls-files -- '*.cpp' '*.h')

clang-format -i -style=file "$files"
