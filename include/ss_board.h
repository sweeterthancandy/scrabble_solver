#pragma once

#include <cassert>
#include <algorithm>
#include <iostream>

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
                board(size_t x = 10, size_t y = 10, char val = ' '):
                        x_len_(x), y_len_(y),
                        rep_(x * y, val)
                {}
                board(board const& that, array_orientation orientation = array_orientation::horizontal):
                        x_len_(that.x_len_), y_len_(that.y_len_)
                        ,rep_(x_len_ * y_len_)
                {
                        for(size_t x=0;x!=x_len_;++x){
                                for(size_t y=0;y!=y_len_;++y){
                                        (*this)(x,y) = that(orientation,x,y);
                                }
                        }
                }
                board(std::vector<std::string> const& rep){
                        if( rep.empty() ){
                                BOOST_THROW_EXCEPTION(std::domain_error("bad rep"));
                        }
                        x_len_ = rep.back().size();
                        y_len_ = rep.size();
                        for(auto const& l : rep ){
                                if( l.size() != x_len_ ){
                                        BOOST_THROW_EXCEPTION(std::domain_error("bad rep"));
                                }
                        }
                        rep_.assign( x_len_ * y_len_, ' ');
                        for( size_t x=0;x!=x_len_;++x){
                                for( size_t y=0;y!=y_len_;++y){
                                        this->operator()(x,y) = rep[x][y];
                                }
                        }
                }

                char operator()(size_t x, size_t y)const{
                        assert( x < x_len_ && "out of bound");
                        assert( y < y_len_ && "out of bound");
                        return rep_.at( x * y_len_ + y);
                }
                char& operator()(size_t x, size_t y){
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



                char operator()(array_orientation orientation, size_t x, size_t y)const{
                        switch(orientation){
                        case array_orientation::horizontal:
                                return this->operator()(x,y);
                        case array_orientation::vertical:
                                return this->operator()(y,x);
                        }
                        __builtin_unreachable();
                }
                char& operator()(array_orientation orientation, size_t x, size_t y){
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
                void fill( char val){
                        boost::fill( rep_, val);
                }

                #if 0
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
                                                ostr << ' ' << ( std::isgraph(d) || std::isspace(d) ? d : '?' ) << ' ';
                                        }
                                }
                                ostr << "|\n";
                        }
                        ostr << std::string((*this).x_len() * 3 + 2, '-') << "\n";
                        ostr << std::flush;
                }
                #else
                void dump(array_orientation orientation = array_orientation::horizontal, std::ostream& ostr = std::cout)const{
                        for(size_t y=0;y!=(*this).y_len(orientation);++y){
                                for(size_t x=0;x!=(*this).x_len(orientation);++x){
                                        auto d = (*this)(orientation,x,y);

                                        switch(d){
                                        case '\0':
                                                ostr << "   ";
                                                break;
                                        default:
                                                ostr << ' ' << ( std::isgraph(d) || std::isspace(d) ? d : '?' ) << ' ';
                                        }
                                }
                                ostr << "\n";
                        }
                }
                #endif
        private:
                size_t x_len_;
                size_t y_len_;
                array_t rep_;
        };

}
