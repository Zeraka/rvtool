#pragma once
#include <string>
#include "util-debug.hh"
#include "util-error.hh"

AMError parse_ltl_file(const std::string& filename, const std::string& fileFormat);

