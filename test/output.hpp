#pragma once
#include <iostream>

const inline auto print = [](const auto& val) { std::cout << val << ' '; };

const inline auto println = [](const auto& val) { std::cout << val << '\n'; };

const inline auto newline = []() { std::cout << '\n'; };
