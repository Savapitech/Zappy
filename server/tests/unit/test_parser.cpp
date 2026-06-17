#include <criterion/criterion.h>

#include "Parser.hpp"

Test(parser, empty_line_yields_no_args) {
  auto args = ParseArgs("");
  cr_assert_eq(args.size(), (size_t)0);
}

Test(parser, splits_on_single_spaces) {
  auto args = ParseArgs("linemate");
  cr_assert_eq(args.size(), (size_t)1);
  cr_assert_str_eq(args[0].c_str(), "linemate");
}

Test(parser, splits_multiple_words) {
  auto args = ParseArgs("hello world foo");
  cr_assert_eq(args.size(), (size_t)3);
  cr_assert_str_eq(args[0].c_str(), "hello");
  cr_assert_str_eq(args[1].c_str(), "world");
  cr_assert_str_eq(args[2].c_str(), "foo");
}

Test(parser, collapses_repeated_whitespace) {
  auto args = ParseArgs("  hello    world  ");
  cr_assert_eq(args.size(), (size_t)2);
  cr_assert_str_eq(args[0].c_str(), "hello");
  cr_assert_str_eq(args[1].c_str(), "world");
}
