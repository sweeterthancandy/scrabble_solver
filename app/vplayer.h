#ifndef VPLAYER_H
#define VPLAYER_H

#include <boost/variant.hpp>

struct skip_go_t{};

struct meta_rewrite{};

struct exchange_t{
        explicit exchange_t(std::string s):s_{std::move(s)}{}
        auto get()const{ return s_; }
private:
        std::string s_;
};

using player_move = boost::variant<
        meta_rewrite,
        skip_go_t,
        exchange_t,
        std::vector<ss::word_placement> 
>;

struct game_context;

struct vplayer{
        virtual ~vplayer()=default;
        virtual player_move exec(game_context& ctx)=0;
        virtual void        post_exec(game_context& ctx){}
};

using vplayer_factory = ss::generic_factory<vplayer>;

struct player_t{
        std::shared_ptr<vplayer> vp;
        std::string backend;
        std::string rack;
        std::vector<std::tuple<size_t, std::string> > score;
        size_t sigma()const{
                size_t result{0};
                for( auto const& s : score ){
                        result += std::get<0>(s);
                }
                return result;
        }
};

#endif // VPLAYER_H
