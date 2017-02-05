#pragma once

#include <iostream>

namespace ss{

        enum class array_orientation{
                horizontal,
                vertical
        };

        inline
        std::ostream& operator<<(std::ostream& ostr, array_orientation orientation){
                switch(orientation){
                case array_orientation::horizontal:
                        return ostr << "horizontal";
                case array_orientation::vertical:
                        return ostr << "vertical";
                }
                return ostr << "(invalid)";
        }
}
