#pragma once

#include <iostream>

namespace ss{

        enum class array_orientation{
                horizontal,
                vertical
        };

        // only makes sense
        inline array_orientation operator!(array_orientation orientation){
                switch(orientation){
                case array_orientation::horizontal:
                        return array_orientation::vertical;
                case array_orientation::vertical:
                        return array_orientation::horizontal;
                }
        }


        inline
        std::ostream& operator<<(std::ostream& ostr, array_orientation orientation){
                switch(orientation){
                case array_orientation::horizontal:
                        return ostr << "horizontal";
                case array_orientation::vertical:
                        return ostr << "vertical";
                }
        }
}
