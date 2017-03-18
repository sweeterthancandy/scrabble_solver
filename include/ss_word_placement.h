#pragma once

namespace ss{
        /*
         * Idea of this is when working out possible moves,
         * some moves have perpendicular words, which during
         * the algorithm the offset (x,y) is already calculated.
         *      Best thing to do is pass a vector of word placements
         * to any consumer of possible moves.
         */
        struct word_placement{
                explicit word_placement(size_t x, size_t y, array_orientation orientation,  std::string word):
                        x_(x),y_(y),
                        orientation_(orientation),
                        word_(std::move(word))
                {}
                auto get_x()const{ return x_; }
                auto get_y()const{ return y_; }
                auto get_orientation()const{ return orientation_; }
                auto get_word()const{ return word_; }
                void dump(std::ostream& ostr = std::cout)const{
                        ostr << "{"
                                << "(" << x_ << "," << y_ << "), "
                                << word_ << "}\n";
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
        };
}
