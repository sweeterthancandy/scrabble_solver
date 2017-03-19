#pragma once

#include <vector>
#include <cstring>

#include "ss_generic_factory.h"

namespace ss{
        struct word_placement;

        struct metric{
                virtual size_t calculate(std::vector<word_placement> const& placements)=0;
        };


        using metric_factory = generic_factory<metric>;
}
