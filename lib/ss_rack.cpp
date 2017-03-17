#include "ss_rack.h"

#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <boost/exception/all.hpp>
#include <sstream>

namespace ss{
        rack::rack(std::vector<tile_t>& tiles):
                tiles_(tiles)
        {
                for(char& c: tiles_)
                        c = toupper(c);
                boost::sort(tiles_);
        }
        rack::rack(std::string const& s)
                :tiles_(s.begin(), s.end()){
                for(char& c: tiles_)
                        c = toupper(c);
                boost::sort(tiles_);
        }
        rack rack::clone_remove_tile(tile_t t)const{
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
        std::set<tile_t> rack::make_tile_set()const{
                std::set<tile_t> tmp;
                boost::for_each( tiles_, [&tmp](auto&& _){ tmp.insert(_); });
                return std::move(tmp);
        }
        size_t rack::size()const{return tiles_.size();}


        std::ostream& operator<<(std::ostream& ostr, rack const& self){
                boost::copy( self.tiles_, std::ostream_iterator<tile_t>(ostr << "{", ","));
                return ostr << "}";
        }
}
