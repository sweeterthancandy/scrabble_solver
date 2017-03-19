#pragma once 

#include "ss_board.h"
#include "ss_driver.h"
#include "ss_generic_factory.h"
#include "ss_renderer.h"
#include "ss_strategy.h"
#include "ss_dict.h"
#include "ss_word_placement.h"
#include "ss_metric.h"

#include <boost/preprocessor.hpp>


#define PRINT(X) do{ std::cout << #X << " = " << (X) << "\n"; }while(0)

#define PRINT_SEQ_detail(r, d, i, e) do{ std::cout << ( i ? ", " : "" ) << BOOST_PP_STRINGIZE(e) << " = " << (e); }while(0);
#define PRINT_SEQ(SEQ) do{ BOOST_PP_SEQ_FOR_EACH_I( PRINT_SEQ_detail, ~, SEQ) std::cout << "\n"; }while(0)

// vim: sw=8 ts=8
