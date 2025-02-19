// Copyright (c) 2020-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <vector>

#include "test.hpp"

namespace tao::pegtl
{
   using grammar = seq< plus< alpha >, star< sor< space, digit > > >;

   template< typename Name >
   struct visitor
   {
      static void visit( std::vector< std::string >& names )
      {
         names.emplace_back( demangle< Name >() );
      }
   };

   void unit_test()
   {
      std::vector< std::string > names;
      visit< grammar, visitor >( names );
      TAO_PEGTL_TEST_ASSERT( names.size() == 7 );
   }

}  // namespace tao::pegtl

#include "main.hpp"
