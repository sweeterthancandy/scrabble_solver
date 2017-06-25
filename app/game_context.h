#ifndef SS_GAME_CONTEXT_H
#define SS_GAME_CONTEXT_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>


#include "ss_board.h"
#include "ss_word_placement.h"
#include "ss_dict.h"
#include "ss_metric.h"

#include "vplayer.h"


enum game_state{
        State_FirstPlacement,
        State_Running,
        State_Finished
};

struct game_context{
        game_context()
                : gen{rd()}
        {}

        void apply_placements(std::vector<ss::word_placement> const& placements);
        void skip_go();
        void exchange(std::string const& s);
        player_t* get_active(){
                return &players[active_player];
        }
        player_t const* get_active()const{
                return &players[active_player];
        }
private:
        void on_finish_();
        void next_();
public:

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
        std::vector<std::string> debug;

        std::string dict;
        std::shared_ptr<ss::dictionary_t> dict_ptr;

        std::string metric;
        std::shared_ptr<ss::metric> metric_ptr;
        
        game_state state;
        size_t skips;
        bool is_rotated;

        int winner;
};

#include "game_context_io.h"


#endif // SS_GAME_CONTEXT_H
