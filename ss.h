#pragma once 

#include <iostream>
#include <map>
#include <sstream>
#include <boost/multi_array.hpp>
#include <boost/exception/all.hpp>
#include <stdexcept>
#include <boost/range/algorithm.hpp>
#include <boost/variant.hpp>
#include <tuple>

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

	/*
	 * I'm going to be using this alot,
	 *
	 */
	template<class T>
	struct generic_factory{
		generic_factory& register_(std::string const& token, std::shared_ptr<T> proto){
                        std::cerr << "registing token=" << token << "\n";
			protos_.insert(std::make_pair(token, std::move(proto)));
			return *this;
		}
		template<class... Args>
		std::shared_ptr<T> make(std::string const& token, Args const... args){
                        
                        boost::for_each( protos_, [&](auto&& _){
                                std::cout << _.first << ",";
                        });
                        std::cout << "\n";
			auto iter = protos_.find(token);
			if( iter == protos_.end() ){
                                std::stringstream sstr;
                                sstr << "unknown token=" << token;
				BOOST_THROW_EXCEPTION(std::domain_error(sstr.str()));
			}
			return iter->second->clone(std::forward<Args>(args)...);
		}

		static generic_factory* get_inst(){
			static std::unique_ptr<generic_factory> mem;
			if( ! mem ){
				mem.reset(new generic_factory);
			}
			return mem.get();
		}
	private:
		generic_factory()=default;
		generic_factory(generic_factory const&)=default;
		generic_factory(generic_factory&&)=default;
		generic_factory& operator=(generic_factory const&)=default;
		generic_factory& operator=(generic_factory&&)=default;
	private:
		std::map<std::string, std::shared_ptr<T> > protos_;
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

	struct renderer{
		virtual void render(board const& board)=0;
                virtual std::shared_ptr<renderer> clone()=0;
	};


        using renderer_factory = generic_factory<renderer>;

        namespace auto_reg{
                std::shared_ptr<renderer> make_cout_renderer(){
                        struct stream_renderer : renderer{
                                explicit stream_renderer(std::ostream& ostr):ostr_(&ostr){}
                                void render(board const& board){
                                        *ostr_ << std::string(board.x_len() * 3 + 2, '-') << "\n";
                                        for(size_t y= board.y_len(); y!= 0;){
                                                --y;
                                                *ostr_ << "|";
                                                for(size_t x=0;x!=board.x_len();++x){
                                                        auto d = board.decoration_at(x,y);
                                                        auto t = board.tile_at(x,y);

                                                        *ostr_ << ' ' << t << ' ';
                                                }
                                                *ostr_ << "|\n";
                                        }
                                        *ostr_ << std::string(board.x_len() * 3 + 2, '-') << "\n";
                                        *ostr_ << std::flush;
                                }
                                std::shared_ptr<renderer> clone(){
                                        return std::make_shared<stream_renderer>(*ostr_);
                                }
                        private:
                                std::ostream* ostr_;
                        };
                        return std::make_shared<stream_renderer>(std::cout);
                }
                int cout_render = (
                       renderer_factory::get_inst()->register_(
                               "cout_renderer",
                              make_cout_renderer()),0);
        }





        

}


// vim: sw=8 ts=8
