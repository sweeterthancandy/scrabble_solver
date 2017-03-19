#include "ss_tile_traits.h"

namespace ss{
        namespace tile_traits{
                bool empty(char c){
                        switch(c){
                        case '\0':
                        case ' ':
                        case '\t':
                                return true;
                        default:
                                return false;
                        }
                }
        }
}
