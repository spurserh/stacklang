set -e
clang++ -std=c++1z  ./parser_test.cc -O0 -g -o /tmp/parser_test
/tmp/parser_test
