#pragma once

#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/range/adaptors.hpp>

#include "ss_generic_factory.h"
#include "ss_rack.h"
#include "ss_orientation.h"

namespace ss{

                

        /*
         * Want to use a lightweight representation of the board,
         * as potentially going to be constructing alot of them
         *
         *
         *      ' '   : nothing
         *      [a-Z] : corresponding tile
         */

        struct board{
                using array_t = std::vector<char>;
                // want to be default constructed
                // TODO make default construction something more logical
                board(size_t x = 10, size_t y = 10, char val = ' ');
                board(board const& that, array_orientation orientation = array_orientation::horizontal);
                explicit board(std::vector<std::string> const& rep);

                char operator()(size_t x, size_t y)const;
                char& operator()(size_t x, size_t y);
		size_t x_len()const{ return x_len_; }
		size_t y_len()const{ return y_len_; }

                char operator()(array_orientation orientation, size_t x, size_t y)const;
                char& operator()(array_orientation orientation, size_t x, size_t y);
		size_t x_len(array_orientation orientation)const;
		size_t y_len(array_orientation orientation)const;
                void fill( char val);
                void dump(array_orientation orientation = array_orientation::horizontal, std::ostream& ostr = std::cout)const;
                std::vector<std::string> to_string_vec()const;
                void rotate_90();
                void transpose();

        private:
                size_t x_len_;
                size_t y_len_;
                array_t rep_;
        };



}
