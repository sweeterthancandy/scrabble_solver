#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "ss_generic_factory.h"

namespace ss{

        using dictionary_t = std::vector<std::string>;
        using dictionary_factory = generic_factory<dictionary_t>;
}
