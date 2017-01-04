#pragma once

#include <boost/multi_array.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/range/adaptors.hpp>
#include <algorithm>
#include <cassert>

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

}
