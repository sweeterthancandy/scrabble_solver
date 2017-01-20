#pragma once

#include <cassert>
#include <algorithm>
#include <iostream>

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/range/adaptors.hpp>

#include "ss_generic_factory.h"
#include "ss_rack.h"

namespace ss{

	enum class decoration{
		none,
		double_letter,
		tripple_letter,
		double_word,
		tripple_word
	};
                
        enum class array_orientation{
                horizontal,
                vertical
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
                basic_array(basic_array const& that, array_orientation orientation = array_orientation::horizontal):
                        x_len_(that.x_len_), y_len_(that.y_len_)
                        ,rep_(x_len_ * y_len_)
                {
                        for(size_t x=0;x!=x_len_;++x){
                                for(size_t y=0;y!=y_len_;++y){
                                        (*this)(x,y) = that(orientation,x,y);
                                }
                        }
                }





                auto const& operator()(size_t x, size_t y)const{
                        assert( x < x_len_ && "out of bound");
                        assert( y < y_len_ && "out of bound");
                        return rep_.at( x * y_len_ + y);
                }
                auto& operator()(size_t x, size_t y){
                        assert( x < x_len_ && "out of bound");
                        assert( y < y_len_ && "out of bound");
                        return rep_.at( x * y_len_ + y);
                }
		size_t x_len()const{
                        return x_len_;
                }
		size_t y_len()const{
                        return y_len_;
                }



                auto const& operator()(array_orientation orientation, size_t x, size_t y)const{
                        switch(orientation){
                        case array_orientation::horizontal:
                                return this->operator()(x,y);
                        case array_orientation::vertical:
                                return this->operator()(y,x);
                        }
                        __builtin_unreachable();
                }
                auto& operator()(array_orientation orientation, size_t x, size_t y){
                        switch(orientation){
                        case array_orientation::horizontal:
                                return this->operator()(x,y);
                        case array_orientation::vertical:
                                return this->operator()(y,x);
                        }
                        __builtin_unreachable();
                }
		size_t x_len(array_orientation orientation)const{
                        switch(orientation){
                        case array_orientation::horizontal:
                                return x_len_;
                        case array_orientation::vertical:
                                return y_len_;
                        }
                        __builtin_unreachable();
                }
		size_t y_len(array_orientation orientation)const{
                        switch(orientation){
                        case array_orientation::horizontal:
                                return y_len_;
                        case array_orientation::vertical:
                                return x_len_;
                        }
                        __builtin_unreachable();
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

                void dump(std::ostream& ostr = std::cout)const{
                        ostr << std::string((*this).x_len() * 3 + 2, '-') << "\n";
                        for(size_t y=0;y!=(*this).y_len();++y){
                                ostr << "|";
                                for(size_t x=0;x!=(*this).x_len();++x){
                                        auto d = (*this)(x,y);

                                        switch(d){
                                        case '\0':
                                                ostr << "   ";
                                                break;
                                        default:
                                                ostr << ' ' << ( std::isgraph(d) ? d : '?' ) << ' ';
                                        }
                                }
                                ostr << "|\n";
                        }
                        ostr << std::string((*this).x_len() * 3 + 2, '-') << "\n";
                        ostr << std::flush;
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

}
