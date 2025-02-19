// Copyright (c) 2020-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"

#include <tao/pegtl/contrib/instantiate.hpp>

namespace tao::pegtl
{
   bool ctor = false;
   bool dtor = false;

   struct test_class
   {
      template< typename ParseInput >
      test_class( const ParseInput& /*unused*/ )
      {
         TAO_PEGTL_TEST_ASSERT( ctor == false );
         TAO_PEGTL_TEST_ASSERT( dtor == false );

         ctor = true;
      }

      test_class( test_class&& ) = delete;
      test_class( const test_class& ) = delete;

      ~test_class()
      {
         TAO_PEGTL_TEST_ASSERT( ctor == true );
         TAO_PEGTL_TEST_ASSERT( dtor == false );

         dtor = true;
      }

      test_class& operator=( test_class&& ) = delete;
      test_class& operator=( const test_class& ) = delete;
   };

   using test_grammar = sor< alpha, digit >;

   template< typename Rule >
   struct test_action
      : nothing< Rule >
   {};

   template<>
   struct test_action< alpha >
   {
      static void apply0()
      {
         TAO_PEGTL_TEST_ASSERT( ctor == true );
         TAO_PEGTL_TEST_ASSERT( dtor == false );
      }
   };

   template<>
   struct test_action< sor< alpha, digit > >
      : instantiate< test_class >
   {};

   void unit_test()
   {
      memory_input in( "a", __FUNCTION__ );
      TAO_PEGTL_TEST_ASSERT( parse< test_grammar, test_action >( in ) );

      TAO_PEGTL_TEST_ASSERT( ctor == true );
      TAO_PEGTL_TEST_ASSERT( dtor == true );
   }

}  // namespace tao::pegtl

#include "main.hpp"
