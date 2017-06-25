#ifndef SS_GAME_CONTEXT_H
#define SS_GAME_CONTEXT_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>

#include <boost/variant.hpp>

#include "ss_board.h"
#include "ss_word_placement.h"
#include "ss_dict.h"
#include "ss_metric.h"

struct skip_go_t{};

struct meta_rewrite{};

using player_move = boost::variant<
        skip_go_t,
        meta_rewrite,
        std::vector<ss::word_placement> 
>;

struct game_context;

struct vplayer{
        virtual ~vplayer()=default;
        virtual player_move exec(game_context& ctx)=0;
};

using vplayer_factory = ss::generic_factory<vplayer>;

struct player_t{
        std::shared_ptr<vplayer> vp;
        std::string backend;
        std::string rack;
        std::vector<unsigned> score;
};

enum game_state{
        State_FirstPlacement,
        State_Running,
        State_Finished
};

struct game_context{
        game_context()
                : gen{rd()}
        {}

        void write(std::ostream& ostr)const;
        void read(std::istream& ostr);
        void render(std::ostream& ostr)const;
        void apply_placements(std::vector<ss::word_placement> const& placements);

        std::random_device rd;
        std::mt19937 gen;

        size_t width;
        size_t height;
        ss::board board;

        std::string bag;
        std::vector<player_t> players;
        size_t active_player;
        std::string scratch = "scrabble.scratch";

        std::vector<std::string> log;
        std::vector<std::string> moves;

        std::string dict;
        std::shared_ptr<ss::dictionary_t> dict_ptr;

        std::string metric;
        std::shared_ptr<ss::metric> metric_ptr;
        
        game_state state;

};

#endif // SS_GAME_CONTEXT_H
