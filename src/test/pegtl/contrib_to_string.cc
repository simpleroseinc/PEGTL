// Copyright (c) 2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/PEGTL/

#include "test.hh"

#include <tao/pegtl.hh>
#include <tao/pegtl/contrib/to_string.hh>

namespace pegtl
{
   void unit_test()
   {
      TEST_ASSERT( to_string< string<> >() == "" );
      TEST_ASSERT(( to_string< string< 'a', 'b', 'c' > >() == "abc" ));

      TEST_ASSERT( to_string< istring<> >() == "" );
      TEST_ASSERT(( to_string< istring< 'a', 'b', 'c' > >() == "abc" ));

      TEST_ASSERT( to_string< tao_pegtl_string_t( "" ) >() == "" );
      TEST_ASSERT( to_string< tao_pegtl_string_t( "abc" ) >() == "abc" );
      TEST_ASSERT( to_string< tao_pegtl_string_t( "AbC" ) >() == "AbC" );
      TEST_ASSERT( to_string< tao_pegtl_string_t( "abc" ) >() != "AbC" );
      TEST_ASSERT( to_string< tao_pegtl_istring_t( "abc" ) >() == "abc" );
      TEST_ASSERT( to_string< tao_pegtl_istring_t( "AbC" ) >() == "AbC" );
      TEST_ASSERT( to_string< tao_pegtl_istring_t( "abc" ) >() != "AbC" );

      // to_string does *not* care about the outer class template
      TEST_ASSERT(( to_string< one< 'a', 'b', 'c' > >() == "abc" ));
   }

} // namespace pegtl

#include "main.hh"