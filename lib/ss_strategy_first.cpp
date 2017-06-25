#include "ss.h"
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
        
namespace ss{
namespace{

        struct first_solver : strategy{

                void yeild(board const& board, rack const& r, dictionary_t const& dict, callback_t callback)override{
                        using std::get;
                        std::vector<std::tuple<std::string, rack> > stack;
                        std::vector<std::string> result;

                        // init stack
                        for( auto t : r.make_tile_set() ){
                                stack.emplace_back( std::string{t}, r.clone_remove_tile(t) );
                        }
                        enum{
                                Ele_Word,
                                Ele_Rack
                        };

                        for( ; stack.size(); ){
                                auto head{ stack.back() };
                                auto const& word{ get<Ele_Word>(head) };
                                auto const& rack{ get<Ele_Rack>(head) };

                                stack.pop_back();

                                if( get<Ele_Word>(head).size() >= 2 ){
                                        if( ! dict.contains_prefix(word) ){
                                                continue;
                                        }
                                        if( dict.contains(word)){
                                                result.push_back(word);
                                        }
                                }
                        
                                for( auto t : rack.make_tile_set() ){
                                        stack.emplace_back( word + t, rack.clone_remove_tile(t) );
                                }
                        }

                        for( auto const& w : result ){
                                std::vector<ss::word_placement> aux;
                                aux.emplace_back(7,7,array_orientation::horizontal,w,w);
                                callback(aux);
                        }

                }
                std::shared_ptr<strategy> clone()override{
                        return std::make_shared<first_solver>();
                }
        };

        int reg = ( strategy_factory::get_inst() ->register_( "first", 
                                                                          [](){
                                                                                return std::make_unique<first_solver>();
                                                                          }
                                                                          ), 0 );
} // anon
} // ss
