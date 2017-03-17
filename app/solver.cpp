#include "ss.h"
#include "ss_util.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


struct ss_context{
};

int main(int argc, char** argv){
        namespace bpt = boost::property_tree;
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
}

// vim: sw=8 ts=8
