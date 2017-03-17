#include "ss.h"
#include "ss_util.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


namespace bpt = boost::property_tree;


struct solver_driver{
        void set_board( ss::board const& proto)                       { board_ = proto; }
        void set_rack( ss::rack const& proto)                         { rack_  = proto; }
        void set_strategy( std::shared_ptr<ss::strategy> proto )      { strat_ = proto; }
        void set_dictionary( std::shared_ptr<ss::dictionary_t> proto ){ dict_  = proto; }

        void run( bpt::ptree& root){
                strat_->yeild( board_, rack_, *dict_,
                              [&](ss::array_orientation orientation,
                                  size_t x, size_t y,
                                  std::string const& word)
                              {
                                      PRINT_SEQ((x)(y)(word));
                                      bpt::ptree child;
                                      child.put("x", x);
                                      child.put("y", y);
                                      child.put("orientation", orientation);
                                      child.put("word", word);
                                      root.add_child("move", child);
                              });
        }

private:
        ss::board board_;
        ss::rack rack_;
        std::shared_ptr<ss::strategy> strat_;
        std::shared_ptr<ss::dictionary_t> dict_;

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

        bpt::ptree ret;
        driver.run(ret);
        bpt::write_json( "result.json", ret);

}

// vim: sw=8 ts=8
