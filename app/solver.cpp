#include "ss.h"
#include "ss_util.h"

#include <tuple>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>



namespace bpt = boost::property_tree;

struct solver_driver{
        void set_board( ss::board const& proto)                       { board_ = proto; }
        void set_rack( ss::rack const& proto)                         { rack_  = proto; }
        void set_strategy( std::shared_ptr<ss::strategy> proto )      { strat_ = proto; }
        void set_dictionary( std::shared_ptr<ss::dictionary_t> proto ){ dict_  = proto; }
        void set_metric( std::shared_ptr<ss::metric> proto )          { metric_= proto; }

        void run( bpt::ptree& root){
                using std::get;
                enum{
                        Ele_X,
                        Ele_Y,
                        Ele_Ori
                };
                using key_type = std::tuple<size_t, size_t, ss::array_orientation>;
                std::map<key_type, std::vector<std::vector<ss::word_placement> > > m;
                strat_->yeild( board_, rack_, *dict_,
                              [&](std::vector<ss::word_placement> const& placements)
                              {
                                      auto const& first{ placements.front() };
                                      m[std::make_tuple(first.get_x(),first.get_y(),first.get_orientation())].emplace_back(placements);
                              });
                // for all (x,y,o)
                for( auto const& p : m ){
                        bpt::ptree moves;
                        

                        moves.put("x", get<Ele_X>(p.first));
                        moves.put("y", get<Ele_Y>(p.first));
                        moves.put("orientation", get<Ele_Ori>(p.first));
                        
                        // for all moves
                        for( auto const& move : p.second){
                                bpt::ptree words;
                                auto metric{ metric_->calculate(move) };
                                for( auto const& placement : move ){
                                        bpt::ptree item;

                                        item.add("x", placement.get_x());
                                        item.add("y", placement.get_y());
                                        item.add("placement", placement.get_word());

                                        words.add_child("placement", item);
                                }
                                words.put("metric", metric);
                                moves.add_child("move", words);
                        }

                        root.add_child("foo", moves);

                        
                }
        }

private:
        ss::board board_;
        ss::rack rack_;
        std::shared_ptr<ss::strategy> strat_;
        std::shared_ptr<ss::dictionary_t> dict_;
        std::shared_ptr<ss::metric> metric_;

};

int main(int argc, char** argv){
        bpt::ptree config;
        bpt::read_json( "data/game0.json", config);

        std::string rack_args{config.get<std::string>("rack")};
        ss::rack r{rack_args};
        r.dump();
        std::vector<std::string> board_args;
        for( auto const& child : config.get_child("board") ){
                board_args.emplace_back( child.second.data() );
        }
        boost::copy( board_args, std::ostream_iterator<std::string>(std::cout, ","));
        ss::board b(board_args);
        b.dump();
        
        solver_driver driver;
        driver.set_board( b );
        driver.set_rack( r );
        driver.set_dictionary( ss::dictionary_factory::get_inst()->make("regular") );
        driver.set_strategy( ss::strategy_factory::get_inst()->make("fast_solver") );
        driver.set_metric( ss::metric_factory::get_inst()->make("scrabble_metric") );

        bpt::ptree ret;
        driver.run(ret);
        bpt::write_json( "result.json", ret);

}

// vim: sw=8 ts=8
