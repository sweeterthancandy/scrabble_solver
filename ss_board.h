#pragma once

#include <boost/multi_array.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/range/adaptors.hpp>
#include <algorithm>
#include <cassert>

#include "ss_generic_factory.h"

namespace ss{

        //
        // [A-Z] -> regular leggers
        // ' '   -> blank
        // [a-z] -> blank assigned to letter
	
        typedef char tile_t;

	enum class decoration{
		none,
		double_letter,
		tripple_letter,
		double_word,
		tripple_word
	};

        struct rack{
                explicit rack(std::vector<tile_t>& tiles):
                        tiles_(tiles){
                        boost::sort(tiles_);
                }
                explicit rack(std::string const& s)
                        :tiles_(s.begin(), s.end()){
                        boost::sort(tiles_);
                }
                rack clone_remove_tile(tile_t t)const{
                        decltype(tiles_) ret;
                        auto pos = boost::find(tiles_, t);
                        auto iter = boost::begin(tiles_);
                        auto end = boost::end(tiles_);
                        if( pos == end ){
                                std::stringstream msg;
                                msg << "no tile " << t << " in rack";
                                BOOST_THROW_EXCEPTION(std::domain_error(msg.str()));
                        }
                        std::copy(iter, pos, std::back_inserter(ret));
                        ++pos;
                        std::copy( pos, end, std::back_inserter(ret));
                        assert( boost::is_sorted(ret) );
                        assert( ret.size() + 1 == this->size() );
                        return rack(ret);
                        
                }
                std::set<tile_t> make_tile_set()const{
                        std::set<tile_t> tmp;
                        boost::for_each( tiles_, [&tmp](auto&& _){ tmp.insert(_); });
                        return std::move(tmp);
                }
                size_t size()const{return tiles_.size();}
        private:
                std::vector<tile_t> tiles_;
        };

        /*
         * Want to use a lightweight representation of the board,
         * as potentially going to be constructing alot of them
         */


        template<class T>
        struct basic_array{
                using value_type = T;
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
                void paste(basic_array const& that, T const& mask){
                        for(size_t x=0;x!=x_len_;++x){
                                for(size_t y=0;y!=y_len_;++y){
                                        if( that(x,y) != mask ){
                                                (*this)(x,y) = that(x,y);
                                        }
                                }
                        }
                }
        private:
                size_t x_len_;
                size_t y_len_;
                array_t rep_;
        };


        template<class Impl>
        struct basic_array_rotate_view{
                using value_type = typename Impl::value_type;
                explicit basic_array_rotate_view(Impl& impl):
                        impl_(&impl)
                {}
                auto const& operator()(size_t x, size_t y)const{
                        return impl_->operator()(y,x);
                }
                auto& operator()(size_t x, size_t y){
                        return impl_->operator()(y,x);
                }
		auto x_len()const{ return impl_->y_len(); }
		auto y_len()const{ return impl_->x_len(); }

                operator basic_array<value_type> const&(){ return *impl_; }
        private:
                Impl* impl_;
        };

        template<class Impl>
        auto make_rotate_view(Impl& impl){
                return basic_array_rotate_view<Impl>(impl);
        }
        template<class Impl>
        auto make_const_rotate_view(Impl const& impl){
                return basic_array_rotate_view<Impl const>(impl);
        }

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
