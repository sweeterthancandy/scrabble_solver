#pragma once

#include <memory>
#include "ss_board.h"

namespace ss{


	struct renderer{
                virtual ~renderer()=default;
                virtual void render(board const& board)=0;
                virtual std::shared_ptr<renderer> clone()=0;
	};


        using renderer_factory = generic_factory<renderer>;

}
