#pragma once

#include <boost/multi_array.hpp>
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

	struct board{
		using board_t = boost::multi_array<decoration, 2>;
		using tiles_t = boost::multi_array<tile_t, 2>;

                // create a blank
		explicit board(size_t x, size_t y):
			board_(boost::extents[x][y]),
			tiles_(boost::extents[x][y])
		{}
		explicit board(board_t const& board, tiles_t const& tiles):
			board_(board), tiles_(tiles)
		{
                        std::fill( tiles_.data(), tiles_.data() + tiles_.num_elements(), ' ' );
                }
		size_t x_len()const{
                        return board_.shape()[0];
                }
		size_t y_len()const{
                        return board_.shape()[1];
                }
		std::shared_ptr<board> clone(){
			return std::make_shared<board>(
				board_, 
				tiles_);
		}
                decoration& decoration_at(size_t x, size_t y){ return board_[x][y]; }
                decoration const& decoration_at(size_t x, size_t y)const{ return board_[x][y]; }
                tile_t& tile_at(size_t x, size_t y){ return tiles_[x][y]; }
                tile_t const& tile_at(size_t x, size_t y)const{ return tiles_[x][y]; }
	private:
		boost::multi_array<decoration, 2> board_;
		boost::multi_array<tile_t, 2>     tiles_;
	};
        
        using board_factory = generic_factory<board>;

	namespace autoreg{
                std::shared_ptr<board> make_plain(){
			auto ptr = std::make_shared<board>(15,15);
			return ptr;
		}
		int plain = ( 
			board_factory::get_inst()->register_(
				"plain",
				make_plain())
			, 0);
	}

}
