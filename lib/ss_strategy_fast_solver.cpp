#include "ss.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"
#include "ss_io.h"

#include <iostream>
#include <fstream>
#include <functional>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>


// this represents the possible

namespace{
        using std::get;
        template<class T, class = std::__void_t< decltype( std::declval<T>().begin() ) > >
        inline std::ostream& dump(std::ostream& ostr, T const& con){
                ostr << "{";
                boost::for_each( con, [&](auto&& _){ ostr << _ << ", ";});
                ostr << "}\n";
                return ostr;
        }

        using namespace ss;

        namespace detail{

                std::vector<std::string> string_cache_lines(array_orientation orientation, board const& b){
                        std::vector<std::string> result;

                        for(size_t j=0;j!=b.y_len(orientation);++j){
                                result.emplace_back();
                                for(size_t i=0;i!=b.x_len(orientation);++i){
                                        result.back() += b(orientation, i,j);
                                }
                        }

                        return result;

                }
        }

        /*
         * It's important to do this in stages rather than one long computation,
         * for easy debugging, ie, can buffer several staging into something pretty
         */
        struct fast_solver : strategy{

                /*
                 This function yeilds the result, by doing the algorithm
                        
                        for every line
                                for all n in 0..|rack|
                                        find the possible sequence T = {t0,t1,..tn}
                                                find all possible words
                 */
                template<class F>
                void solve_(board const& brd, array_orientation orientation, rack const& rck, dictionary_t const& dict, F f)
		{

                                
                        auto board_lines = detail::string_cache_lines(orientation, brd);

                        assert( board_lines.size() && "precondition failed");
                        
                        

                        size_t width = board_lines.front().size();

                        for(size_t i=0;i!=board_lines.size();++i){

                                std::string current_line = board_lines[i];

                                /*
                                Need to cache the sequence of moves, which is nice because it is just the 
                                sequence of empty tiles

                                Also cache an auxiallar sum, so I can tell is a subsequence is a valid move
                                 */
                                std::vector<std::tuple<int, int, std::string, std::string> > moves;
                                enum{
                                        Ele_Idx,
                                        Ele_Start,
                                        Ele_Left,
                                        Ele_Right
                                };
                                std::vector<int> sum;
                                int sigma = 0;


                                /*
                                   First we can't to find the smallest set of tiles such that every 
                                   possible must contain at least one tile from this set. This means 
                                   that not only is the tile blank, but it's also perpendular to another
                                   tile, as per the rules of scrabble.

                                                                0123456789
                                                               +----------+
                                                               |          | 0
                                                               | HELLO    | 1
                                                               |   WORLD  | 2
                                                               |          | 3
                                                               +----------+

                                        Consider the board about, where I've labeled the lines 0,1,2,3.
                                   For the first row, although all the tiles are empty, only those tiles
                                   {1,2,3,4,5} are perpenduclar are "start moves". As for the second row,
                                   tiles {0,6,7}, {1,2,8} on the third, and {3,4,5,6,7} for the last row.
                                   Which means we have the following bitmask.

                                                                0123456789
                                                               +----------+
                                                               | XXXXX    | 0
                                                               |X     XX  | 1
                                                               | XX     X | 2
                                                               |   XXXXX  | 3
                                                               +----------+

                                        From this, can compute all the possible moves in terms of where they can start
                                   from for how long. As consider any subset of n blank tiles. This is a possible move
                                   if one of the blank tiles is one of those marked above. 
                                   

                                 */


                                for(size_t j=0;j!=width;++j){
                                        if( current_line[j] != '\0')
                                                continue;
                                        // this is a possible move
                                        std::string left;
                                        std::string right;
                                        for(size_t k=i;k!=0;){
                                                --k;
                                                if( board_lines[k][j] == '\0'){
                                                        break;
                                                }
                                                left += board_lines[k][j];
                                        }
                                        for(size_t k=i+1;k < width;++k){
                                                if( board_lines[k][j] == '\0'){
                                                        break;
                                                }
                                                right += board_lines[k][j];
                                        }
                                        int is_start = ( j     != 0           && current_line[j-1] != '\0' ) ||
                                                       ( j + 1 != current_line.size() && current_line[j+1] != '\0' ) ||
                                                       left.size() || 
                                                       right.size();

                                        left = std::string(left.rbegin(),left.rend());
                                        moves.emplace_back( std::forward_as_tuple( j, is_start, std::move(left), std::move(right)));
                                        sum.emplace_back(sigma);
                                        sigma += is_start;
                                }
                                sum.emplace_back(sigma);

                                /*

                                   Now I need to figure out, for each starting position i (this means the first tile
                                   place in the form, so for example for

                                                                0123456789
                                                               +----------+
                                                               |          | 0
                                                               | HELLO    | 1
                                                               |   WORLD  | 2
                                                               |          | 3
                                                               +----------+

                                        We can see the starts in the form above. Now that for prepending a word, first
                                   starting position is the first blank you place down.

                                                                0123456789
                                                               +----------+
                                                               |XXXXXX    | 0
                                                               |X     X   | 1
                                                               |  X     X | 2
                                                               |   XXXXX  | 3
                                                               +----------+

                                        This calculation is only important, for where the number of tiles placed must be at least
                                   n_{min} to be a valid move.

                                 */
                                
                                // start, number of tiles to put down
                                #if 0
                                std::vector<std::tuple<size_t, std::vector<size_t> > > start_vecs;
                                for(size_t j=0;j + n <= moves.size(); ++j){
                                        std::vector<int> len;
                                        for(size_t n=1;n <= rck.size();++n){
                                                auto diff = sum[j+n] - sum[j];
                                                if(diff){
                                                        len.push_back(n);
                                                }
                                        }
                                        if( len.size() ){
                                                start_vecs.push_back( std::make_tuple(j, std::move(len)));
                                        }

                                }
                                #else
                                #endif

                                std::vector<std::tuple<size_t, std::vector<int> > > start_vecs;

                                for(size_t n=1;n <= rck.size();++n){

                                        std::vector<int> start_vec;

                                        for(size_t j=0;j + n <= moves.size(); ++j){
                                                auto diff = sum[j+n] - sum[j];
                                                if(diff){
                                                        start_vec.push_back(j);
                                                }
                                        }
                                        start_vecs.emplace_back(std::make_tuple(n, std::move(start_vec)));
                                }

                                for( auto const& t : start_vecs){
                                        auto n = get<0>(t);
                                        auto start_vec = get<1>(t);

                                        for( int start : start_vec ){


                                                auto const& start_move(moves[start]);
                                                std::string prefix;
                                                for( size_t j= get<Ele_Idx>(start_move); j != 0; ){
                                                        --j;
                                                        if( current_line[j] == '\0')
                                                                break;
                                                        prefix += current_line[j];
                                                }
                                                prefix = std::string(prefix.rbegin(), prefix.rend());
                                                
                                                io::board_renderer r(brd, orientation);
                                                r.title("starting solve_")
                                                        .mark_row(i)
                                                        .put_tag("start", start)
                                                        .put_tag("start_move", get<Ele_Idx>(start_move))
                                                        .put_tag("prefix", prefix)
                                                        .put_tag("n", n)
                                                        ;


                                                std::vector<std::tuple<std::string, size_t, rack> > stack;
                                                enum{
                                                        Item_Word,
                                                        Item_MoveIdx,
                                                        Item_Rack,
                                                        Item_Depth
                                                };

                                                stack.emplace_back( std::move(prefix), start, rck);
                                                for(; stack.size();){
                                                        auto item = stack.back();
                                                        stack.pop_back();
                                                        auto current_idx {get<Item_MoveIdx>(item)};
                                                        auto delta{current_idx - start};
                                                
                                                        auto cmt = [&](std::string const& comment){
                                                                std::string s;
                                                                if( delta){
                                                                        s += std::string(delta*2 ,'-');
                                                                }
                                                                s += comment;
                                                                s += "(delta=" + boost::lexical_cast<std::string>(delta) + ")";
                                                                r.comment(s);
                                                        };

                                                        if( ! dict.contains_prefix( get<Item_Word>(item) )){
                                                                continue;
                                                        }


                                                                

                                                        //PRINT_SEQ((get<Item_Word>(item))(get<Item_MoveIdx>(item))(get<Item_Rack>(item)));

                                                        if( delta == n ){
                                                                // terminal
                                                                auto word = get<Item_Word>(item);

                                                                bool ret = dict.contains(word);

                                                                //PRINT_SEQ((ret)(i)(n)(start)(word));

                                                                if( ret ){
                                                                        cmt( "found word : " + word);
                                                                }else{
                                                                        cmt( "not a word : " + word);
                                                                }

                                                                if( ret ){
                                                                        f(start, i, word);
                                                                }
                                                                
                                                        } else{
                                                                // else, yeild all other possible states
                                                                
                                                                auto current_rack{get<Item_Rack>(item)};
                                                                auto const& current_move{moves.at(current_idx)};
                                                                std::string current_move_suffix{get<Ele_Right>(current_move)};
                                                                std::string current_move_prefix{get<Ele_Left>(current_move)};
                                                                std::string suffix;

                                                                //PRINT_SEQ((current_move_prefix)(current_move_suffix));

                                                                if( current_idx + 1 < moves.size() ){

                                                                        auto cpy_start{get<Ele_Idx>(moves[current_idx]) +1},
                                                                             cpy_end  {get<Ele_Idx>(moves[current_idx +1])};
                                                                        //PRINT_SEQ((cpy_start)(cpy_end));
                                                                        for(;
                                                                            cpy_start!=cpy_end;
                                                                            ++cpy_start)
                                                                        {
                                                                                suffix += current_line[cpy_start];
                                                                        }
                                                                }


                                                                for( auto t : current_rack.make_tile_set() ){

                                                                        if( current_move_suffix.size() || current_move_prefix.size() ){
                                                                                auto perp_word{current_move_prefix};
                                                                                perp_word += t;
                                                                                perp_word += current_move_suffix;
                                                                                bool ret = dict.contains(perp_word);
                                                                                //PRINT_SEQ((ret)(perp_word));
                                                                                if( ret ){
                                                                                        cmt( "found perp word : " + perp_word);
                                                                                }else{
                                                                                        cmt( "not a perp word : " + perp_word);
                                                                                }
                                                                                if( ! ret ){
                                                                                        continue;
                                                                                }
                                                                        }

                                                                        std::string next_suffix{ get<Item_Word>(item) + t + suffix };

                                                                        stack.emplace_back(
                                                                                next_suffix,
                                                                                get<Item_MoveIdx>(item)+1,
                                                                                current_rack.clone_remove_tile(t));

                                                                }
                                                        }
                                                }
                                                r.display();
                                        }
                                }
                        }
                }
                void yeild(board const& board, rack const& r, dictionary_t const& dict, callback_t callback)override{
                        for( auto orientation : std::vector<array_orientation>{array_orientation::horizontal, array_orientation::vertical} ){
                                std::cout << "solving " << orientation << "\n";

                                this->solve_(board, orientation, r, dict, [&](size_t x, size_t y, std::string const& word){
                                             switch(orientation){
                                             case array_orientation::vertical:
                                                     callback(orientation, y, x, word);
                                                     break;
                                             default:
                                                     callback(orientation, x, y, word);
                                                     break;
                                            }
                                });
                        }
                }
                std::shared_ptr<strategy> clone()override{
                        return std::make_shared<fast_solver>();
                }
        };
        
        
        int reg_brute_force = ( strategy_factory::get_inst() ->register_( "fast_solver", 
                                                                          [](){
                                                                                return std::make_unique<fast_solver>();
                                                                          }
                                                                          ), 0 );

}
