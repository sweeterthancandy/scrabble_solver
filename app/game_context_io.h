#ifndef SS_GAME_CONTEXT_IO_H
#define SS_GAME_CONTEXT_IO_H

#include "game_context.h"

struct game_context_io{
        void read(game_context& ctx, std::istream& ostr)const;
        void write(game_context const& ctx, std::ostream& ostr)const;
        void render(game_context const& ctx, std::ostream& ostr)const;
        void write_all(game_context const& ctx)const;
};

#endif // SS_GAME_CONTEXT_IO_H 
