// Copyright (c) 2020-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include "test.hpp"

#include <tao/pegtl/contrib/print.hpp>

namespace tao::pegtl
{
   using grammar = seq< alpha, digit >;

   void unit_test()
   {
      // Just enough to see that it compiles and nothing explodes;
      // the output format probabaly changes between compilers and
      // versions making a proper test difficult.
      print_names< grammar >( std::cout );
      print_debug< grammar >( std::cout );
   }

}  // namespace tao::pegtl

#include "main.hpp"
