#pragma once

#include <string>
#include <vector>

std::vector<std::string> Parse_label_exp_to_RPN(std::string &label);
std::vector<std::string> Parse_label_RPN_to_string_sets(std::vector<std::string> &rpn);