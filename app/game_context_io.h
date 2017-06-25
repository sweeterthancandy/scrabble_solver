#ifndef SS_GAME_CONTEXT_IO_H
#define SS_GAME_CONTEXT_IO_H

#include <boost/optional.hpp>
#include "game_context.h"

namespace tc{
        struct placeholder;
        struct above_below_composite;
} // tc

struct game_context_io{
        game_context_io();
        void read(game_context& ctx, std::istream& ostr)const;
        void write(game_context const& ctx, std::ostream& ostr)const;
        void render(game_context const& ctx, std::ostream& ostr)const;
        void render_better(game_context const& ctx, std::ostream& ostr)const;
        void write_all(game_context const& ctx)const;

        struct parse_result_t{
                ss::board   b;
                std::string r;
        };

        boost::optional<parse_result_t> parse(game_context const& ctx, std::istream& istr)const;
private:
        std::shared_ptr<tc::placeholder>           title_;
        std::shared_ptr<tc::placeholder>           board_;
        std::shared_ptr<tc::placeholder>           rack_;
        std::shared_ptr<tc::placeholder>           score_;
        std::shared_ptr<tc::above_below_composite> root_;
};

#endif // SS_GAME_CONTEXT_IO_H 
