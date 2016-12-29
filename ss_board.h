#pragma once

#include <boost/multi_array.hpp>
#include <boost/range/algorithm.hpp>
#include <algorithm>

#include "ss_generic_factory.h"

namespace ss{

	typedef char tile_t;

	enum class decoration{
		none,
		double_letter,
		tripple_letter,
		double_word,
		tripple_word
	};

        /*
         * Want to use a lightweight representation of the board,
         * as potentially going to be constructing alot of them
         */


        template<class T>
        struct basic_array{
                using array_t = std::vector<T>;
                basic_array(size_t x, size_t y, T const& val = T()):
                        x_len_(x), y_len_(y),
                        rep_(x * y, val)
                {}
                auto const& operator()(size_t x, size_t y)const{
                        return rep_.at( x * y_len_ + y);
                }
                auto& operator()(size_t x, size_t y){
                        return rep_.at( x * y_len_ + y);
                }
		size_t x_len()const{
                        return x_len_;
                }
		size_t y_len()const{
                        return y_len_;
                }
                void fill( T const& val){
                        boost::fill( rep_, val);
                }
                std::shared_ptr<basic_array> clone(){
                        return std::make_shared<basic_array>(*this);
                }
        private:
                size_t x_len_;
                size_t y_len_;
                array_t rep_;
        };

        using board = basic_array<tile_t>;
        using score_board = basic_array<decoration>;


        using board_factory = generic_factory<board>;
        using score_board_factory = generic_factory<score_board>;

	namespace autoreg{
                std::shared_ptr<score_board> make_plain_score(){
			auto ptr = std::make_shared<score_board>(15,15, decoration::none );
			return ptr;
		}
		int plain_score = ( 
			score_board_factory::get_inst()->register_(
				"plain",
				make_plain_score())
			, 0);
                std::shared_ptr<board> make_plain(){
			auto ptr = std::make_shared<board>(15,15, '\0');
			return ptr;
		}
		int plain = ( 
			board_factory::get_inst()->register_(
				"plain",
				make_plain())
			, 0);
	}
}
