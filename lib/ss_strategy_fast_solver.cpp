#include "ss.h"
#include "ss_print.h"
#include "ss_board.h"
#include "ss_driver.h"
#include "ss_dict.h"
#include "ss_util.h"
#include "ss_io.h"
#include "ss_word_placement.h"
#include "ss_tile_traits.h"

#include <iostream>
#include <fstream>
#include <functional>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//#define ALGORITHM_DEBUG

#if 1
#ifdef  PRINT_SEQ
#undef  PRINT_SEQ
#endif /* PRINT_SEQ */

#define PRINT_SEQ(SEQ) 
#endif

// this represents the possible
        using std::get;
        template<class T, class = std::__void_t< decltype( std::declval<T>().begin() ) > >
        inline std::ostream& dump(std::ostream& ostr, T const& con){
                ostr << "{";
                boost::for_each( con, [&](auto&& _){ ostr << _ << ", ";});
                ostr << "}\n";
                return ostr;
        }

        using namespace ss;


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
                        namespace bpt = boost::property_tree;
                        bpt::ptree debug_root;

                                
                        auto board_lines = string_cache_lines(orientation, brd);

                        std::function<word_placement(size_t, size_t, std::string)>
                                make_placement,
                                make_perp_placement
                        ;
                        switch( orientation ){
                        case array_orientation::horizontal:
                                make_placement = [&](size_t x, size_t y, std::string word){
                                        return make_word_placement(brd, x,y, array_orientation::horizontal, word);
                                };
                                make_perp_placement = [&](size_t x, size_t y, std::string word){
                                        return make_word_placement(brd, x,y, array_orientation::vertical, word);
                                };
                                break;
                        case array_orientation::vertical:
                                make_placement = [&](size_t x, size_t y, std::string word){
                                        return make_word_placement(brd,y,x, array_orientation::vertical, word);
                                };
                                make_perp_placement = [&](size_t x, size_t y, std::string word){
                                        return make_word_placement(brd,y,x, array_orientation::horizontal, word);
                                };
                                break;
                        }


                        assert( board_lines.size() && "precondition failed");
                        
                        size_t width = board_lines.front().size();

                        PRINT_SEQ((width));

                        for(size_t i=0;i!=board_lines.size();++i){

                                std::string current_line = board_lines[i];
                                PRINT_SEQ((current_line));

                                /*
                                Need to cache the sequence of moves, which is nice because it is just the 
                                sequence of empty tiles

                                Also cache an auxiallar sum, so I can tell is a subsequence is a valid move
                                 */


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
                                   

                                        Also cache the prefix, ie, doe (4,2), we have left = TW, right = N

                                                                0123456789
                                                               +----------+
                                                               |    T     | 0
                                                               |    W     | 1
                                                               |          | 2
                                                               |    N     | 3
                                                               +----------+

                                   

                                 */

                                std::vector<std::tuple<int, int, std::string, std::string, std::string, std::string> > moves;
                                enum{
                                        Ele_Idx,
                                        Ele_Start,
                                        Ele_Left,
                                        Ele_Right,
                                        Ele_Prefix,
                                        Ele_Suffix 
                                };
                                std::vector<int> sum{0};


                                PRINT_SEQ((i)(board_lines[i]));
                                for(size_t j=0;j!=width;++j){
                                        if( tile_traits::not_empty(board_lines[i][j]) )
                                                continue;
                                        // this is a possible move
                                        std::string left;
                                        std::string right;
                                        

                                        for(size_t k=i;k!=0;){
                                                --k;
                                                if( tile_traits::empty(board_lines[k][j])){
                                                        break;
                                                }
                                                left += board_lines[k][j];
                                        }
                                        for(size_t k=i+1;k < width;++k){
                                                if( tile_traits::empty(board_lines[k][j])){
                                                        break;
                                                }
                                                right += board_lines[k][j];
                                        }
                                        
                                        // suffix is tiles right of this move, is empty when the
                                        // next is a blan
                                        std::string suffix;
                                        std::string prefix;
                                        for(size_t k=j;k!=0;){
                                                --k;
                                                if( tile_traits::empty(board_lines[i][k]))
                                                        break;
                                                prefix += board_lines[i][k];
                                        }
                                        for(size_t k=j+1;k  <width;++k){
                                                if( tile_traits::empty(board_lines[i][k]))
                                                        break;
                                                suffix += board_lines[i][k];
                                        }


                                        int is_start = ( j     != 0           && tile_traits::not_empty(current_line[j-1]) ) ||
                                                       ( j + 1 != current_line.size() && tile_traits::not_empty(current_line[j+1]) ) ||
                                                       left.size() || 
                                                       right.size();

                                        PRINT_SEQ((j)(left)(left)(suffix)(prefix)(is_start));

                                        left = std::string(left.rbegin(),left.rend());
                                        prefix = std::string(prefix.rbegin(),prefix.rend());

                                        moves.emplace_back( std::forward_as_tuple( j, is_start,
                                                                                   std::move(left), std::move(right), 
                                                                                   std::move(prefix),std::move(suffix)));


                                        // also calculate the sigma
                                        sum.emplace_back(sum.back() + is_start);
                                }

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
                                
                                // {start, min tiles}
                                std::vector<std::tuple<size_t, size_t> > start_vecs;
                                //for(size_t j=0;j + n <= moves.size(); ++j){
                                for(size_t j=0;j < moves.size(); ++j){
                                        // as word of lengh n long enough to cover a start tile?
                                        for(size_t n=1;
                                            j+n <= moves.size() && n <= rck.size();
                                            ++n){
                                                auto diff = sum[j+n] - sum[j];
                                                if(diff){
                                                        start_vecs.push_back(std::make_tuple(j,n));
                                                        break;
                                                }
                                        }
                                }

                                #ifdef ALGORITHM_DEBUG
                                std::cout << "start_vecs = {";
                                for( auto const& p : start_vecs)
                                        std::cout << "(" << get<0>(p) << ", " << get<1>(p) << "), ";
                                std::cout << "}\n";
                                #endif


                                for( auto const& t : start_vecs){

                                        auto move_idx = get<0>(t);
                                        auto min_n = get<1>(t);

                                        auto const& start_move(moves[move_idx]);
                                        std::string prefix{ get<Ele_Prefix>(start_move) };

                                        auto word_start = get<Ele_Idx>(start_move) - prefix.size();

                                        PRINT_SEQ((move_idx)(min_n)(prefix));
                                        
                                        std::vector<
                                                std::tuple<
                                                        std::string,
                                                        std::vector<word_placement>,
                                                        size_t,
                                                        rack
                                                >
                                        > stack;
                                        enum{
                                                Item_Word,
                                                Item_Perps,
                                                Item_MoveIdx,
                                                Item_Rack
                                        };

                                        stack.emplace_back( prefix, std::vector<word_placement>{}, move_idx, rck);

                                        for(; stack.size();){

                                                auto item = stack.back();
                                                stack.pop_back();

                                                auto current_idx {get<Item_MoveIdx>(item)};
                                                // how many tiles have i placed?
                                                auto delta{current_idx - move_idx};
                                        
                                                auto cmt = [&](std::string const& comment){
                                                        #ifdef ALGORITHM_DEBUG
                                                        std::string s;
                                                        if( delta){
                                                                s += std::string(delta*2 ,'-');
                                                        }
                                                        s += comment;
                                                        s += "(delta=" + boost::lexical_cast<std::string>(delta) + ")";
                                                        std::cout << s << "\n";
                                                        #endif // ALGORITHM_DEBUG
                                                };

                                                // check a least the prefix is valid
                                                if( get<Item_Word>(item).size() >= 2 && ! dict.contains_prefix( get<Item_Word>(item) )){
                                                        cmt("not a prefix " + get<Item_Word>(item));
                                                        continue;
                                                }


                                                        

                                                //PRINT_SEQ((get<Item_Word>(item))(get<Item_MoveIdx>(item))(get<Item_Rack>(item)));

                                                // is this enought to be a valid word
                                                if( delta >= min_n ){
                                                        // terminal
                                                        auto word = get<Item_Word>(item);
                                                        PRINT_SEQ((word));

                                                        bool ret = dict.contains(word);

                                                        //PRINT_SEQ((ret)(i)(n)(move_idx)(word));

                                                        if( ret ){
                                                                cmt( "found word : " + word);
                                                        }else{
                                                                cmt( "not a word : " + word);
                                                        }

                                                        if( ret ){
                                                                std::vector<word_placement> placements;
                                                                placements.push_back(
                                                                        make_placement(
                                                                                word_start,
                                                                                i, word));
                                                                boost::copy( get<Item_Perps>(item), std::back_inserter(placements));

                                                                f(placements);
                                                        }
                                                        
                                                } 
                                                
                                                if( current_idx == moves.size() )
                                                        continue;
                                                // else, yeild all other possible states
                                                
                                                auto current_rack{get<Item_Rack>(item)};
                                                auto const& current_move{moves.at(current_idx)};

                                                /* These are for perpinducatlor words, ie for below
                                                   placeing WORLD creates the perpenduclar word
                                                   TOO, with the prefix T and the suffic O,
                                                  
                                                                0123456789
                                                               +----------+
                                                               |   T      | 0
                                                              >|  WORLD   | 1
                                                               |   0      | 2
                                                               |          | 3
                                                               +----------+
                                                                   ^
                                                   but of couse would be no suffix, or prefix, consider
                                                   placing WORLD below, we create the perp world AW and 
                                                   the perp word LA, with the formar prefix A (suffix null)
                                                   and the latter with suffic A.

                                                                0123456789
                                                               +----------+
                                                               |  A       | 0
                                                               |  WORLD   | 1
                                                               |     A    | 2
                                                               |          | 3
                                                               +----------+
                                                                    

                                                */



                                                /*
                                                   Consider placeing letters HE incremenalled on the 
                                                   board,
                                                                0123456789
                                                               +----------+
                                                               |    T     | 0
                                                               |    A     | 1
                                                               |    L     | 2
                                                               |    L     | 3
                                                               +----------+
                                                   which will create the board HEL, 
                                                   
                                                                0123456789
                                                               +----------+
                                                               |    T     | 0
                                                               |    A     | 1
                                                               |  HEL     | 2
                                                               |    L     | 3
                                                               +----------+
                                                   ie the word is at HEL after placing HE, with the suffix
                                                   L. Ie placing HELO creates HELLO.

                                                   
                                                 */

                                                std::string current_move_right{get<Ele_Right>(current_move)};
                                                std::string current_move_left{get<Ele_Left>(current_move)};
                                                std::string current_move_prefix{get<Ele_Prefix>(current_move)};
                                                std::string current_move_suffix{get<Ele_Suffix>(current_move)};

                                                for( auto t : current_rack.make_tile_set() )
                                                {
                                                        std::string next_suffix{ get<Item_Word>(item) + t + current_move_suffix };

                                                        std::string perp_word;
                                                        auto perps = get<Item_Perps>(item);

                                                        if( current_move_right.size() || current_move_left.size() ){
                                                                perp_word = current_move_left;
                                                                perp_word += t;
                                                                perp_word += current_move_right;
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

                                                        if( perp_word.size() ){
                                                                perps.emplace_back(
                                                                        make_perp_placement( 
                                                                                word_start + get<Item_Word>(item).size(),
                                                                                i - current_move_left.size(),
                                                                                perp_word) );
                                                        }

                                                        stack.emplace_back(
                                                                next_suffix,
                                                                std::move(perps),
                                                                get<Item_MoveIdx>(item)+1,
                                                                current_rack.clone_remove_tile(t));

                                                }
                                        }
                                }
                        }
                }
                void yeild(board const& board, rack const& r, dictionary_t const& dict, callback_t callback)override{
                        for( auto orientation : std::vector<array_orientation>{array_orientation::horizontal, array_orientation::vertical} ){
                                this->solve_(board, orientation, r, dict, callback);
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

