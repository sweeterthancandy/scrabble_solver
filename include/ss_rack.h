#pragma once

#include "ss_tile.h"

#include <string>
#include <vector>
#include <set>

namespace ss{
        struct rack{
                rack()=default;
                explicit rack(std::vector<tile_t>& tiles);
                explicit rack(std::string const& s);
                rack clone_remove_tile(tile_t t)const;
                std::set<tile_t> make_tile_set()const;
                size_t size()const;
        private:
                std::vector<tile_t> tiles_;
        };
}
