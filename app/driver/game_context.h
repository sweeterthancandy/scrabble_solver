#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <random>

#include "ss_board.h"
#include "ss_word_placement.h"
#include "ss_dict.h"
#include "ss_metric.h"

struct player_t{
        std::string backend;
        std::string rack;

};

enum class game_state{
        running,
        less_than_7_tiles,
        finish,
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

};
