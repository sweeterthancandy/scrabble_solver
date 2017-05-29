#pragma once

#include "ss_orientation.h"
#include "ss_tile_traits.h"

namespace ss{

        struct board;
        /*
         * Idea of this is when working out possible moves,
         * some moves have perpendicular words, which during
         * the algorithm the offset (x,y) is already calculated.
         *      Best thing to do is pass a vector of word placements
         * to any consumer of possible moves.
         */
        struct word_placement{
                /*
                        Consider placing the tiles {T,N,E} below
                        to make the word TONE,

                            0123456789           0123456789
                           +----------+         +----------+
                           |    T     | 0       |    T     | 0
                           |    W     | 1       |    W     | 1
                           |    O     | 2      >|   TONE   | 2
                           |          | 3       |          | 3
                           +----------+         +----------+
                                                    ^
                        word_placement(3,2,"TONE", "T NE")
               */
                explicit word_placement(size_t x, size_t y,
                                        array_orientation orientation,
                                        std::string word,
                                        std::string mask):
                        x_(x),y_(y),
                        orientation_(orientation),
                        word_(std::move(word)),
                        mask_(std::move(mask))
                {}
                auto get_x()const{ return x_; }
                auto get_y()const{ return y_; }
                auto get_orientation()const{ return orientation_; }
                auto get_word()const{ return word_; }
                auto get_mask()const{ return mask_; }
                void dump(std::ostream& ostr = std::cout)const{
                        ostr << "{"
                                << "(" << x_ << "," << y_ << "), "
                                << word_ << ", " << mask_ << "}\n";
                }
                friend std::ostream& operator<<(std::ostream& ostr, word_placement const& self){
                        self.dump(ostr);
                        return ostr;
                }
        private:
                size_t x_;
                size_t y_;
                array_orientation orientation_;
                std::string word_;
                std::string mask_;
        };

        word_placement make_word_placement(board const& b, size_t x, size_t y, array_orientation orientation, std::string word);
}
