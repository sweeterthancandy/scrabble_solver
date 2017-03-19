#pragma once

#include <vector>
#include <cstring>

#include "ss_generic_factory.h"
#include "ss_word_placement.h"

namespace ss{

        struct metric{
                virtual size_t calculate(word_placement const& p){
                        return this->calculate(std::vector<word_placement>{p});
                }
                virtual size_t calculate(std::vector<word_placement> const& placements)=0;
        };


        using metric_factory = generic_factory<metric>;
}
