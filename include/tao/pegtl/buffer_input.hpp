// Copyright (c) 2016-2022 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_BUFFER_INPUT_HPP
#define TAO_PEGTL_BUFFER_INPUT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#if defined( __cpp_exceptions )
#include <stdexcept>
#else
#include <cstdio>
#include <exception>
#endif

#include "eol.hpp"
#include "memory_input.hpp"
#include "position.hpp"
#include "tracking_mode.hpp"

#include "internal/action_input.hpp"
#include "internal/bump.hpp"
#include "internal/frobnicator.hpp"
#include "internal/rewind_guard.hpp"

namespace tao::pegtl
{
   template< typename Reader, typename Eol = eol::lf_crlf, typename Source = std::string, std::size_t Chunk = 64 >
   class buffer_input
   {
   public:
      using reader_t = Reader;

      using eol_t = Eol;
      using source_t = Source;

      using frobnicator_t = internal::frobnicator;

      using action_t = internal::action_input< buffer_input >;

      static constexpr std::size_t chunk_size = Chunk;
      static constexpr tracking_mode tracking_mode_v = tracking_mode::eager;

      template< typename T, typename... As >
      buffer_input( T&& in_source, const std::size_t maximum, As&&... as )
         : m_reader( std::forward< As >( as )... ),
           m_maximum( maximum + Chunk ),
           m_buffer( new char[ maximum + Chunk ] ),
           m_current( m_buffer.get() ),
           m_end( m_buffer.get() ),
           m_source( std::forward< T >( in_source ) )
      {
         static_assert( Chunk != 0, "zero chunk size not implemented" );
         assert( m_maximum > maximum );  // Catches overflow; change to >= when zero chunk size is implemented.
      }

      buffer_input( const buffer_input& ) = delete;
      buffer_input( buffer_input&& ) = delete;

      ~buffer_input() = default;

      buffer_input& operator=( const buffer_input& ) = delete;
      buffer_input& operator=( buffer_input&& ) = delete;

      [[nodiscard]] bool empty()
      {
         require( 1 );
         return m_current.data == m_end;
      }

      [[nodiscard]] std::size_t size( const std::size_t amount )
      {
         require( amount );
         return buffer_occupied();
      }

      [[nodiscard]] const char* current() const noexcept
      {
         return m_current.data;
      }

      [[nodiscard]] const char* end( const std::size_t amount )
      {
         require( amount );
         return m_end;
      }

      [[nodiscard]] std::size_t byte() const noexcept
      {
         return m_current.byte;
      }

      [[nodiscard]] std::size_t line() const noexcept
      {
         return m_current.line;
      }

      [[nodiscard]] std::size_t column() const noexcept
      {
         return m_current.column;
      }

      [[nodiscard]] const Source& source() const noexcept
      {
         return m_source;
      }

      [[nodiscard]] char peek_char( const std::size_t offset = 0 ) const noexcept
      {
         return m_current.data[ offset ];
      }

      [[nodiscard]] std::uint8_t peek_uint8( const std::size_t offset = 0 ) const noexcept
      {
         return static_cast< std::uint8_t >( peek_char( offset ) );
      }

      void bump( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump( m_current, in_count, Eol::ch );
      }

      void bump_in_this_line( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump_in_this_line( m_current, in_count );
      }

      void bump_to_next_line( const std::size_t in_count = 1 ) noexcept
      {
         internal::bump_to_next_line( m_current, in_count );
      }

      void discard() noexcept
      {
         if( m_current.data > m_buffer.get() + Chunk ) {
            const auto s = m_end - m_current.data;
            std::memmove( m_buffer.get(), m_current.data, s );
            m_current.data = m_buffer.get();
            m_end = m_buffer.get() + s;
         }
      }

      void require( const std::size_t amount )
      {
         if( m_current.data + amount <= m_end ) {
            return;
         }
         if( m_current.data + amount > m_buffer.get() + m_maximum ) {
#if defined( __cpp_exceptions )
            throw std::overflow_error( "require() beyond end of buffer" );
#else
            std::fputs( "overflow error: require() beyond end of buffer\n", stderr );
            std::terminate();
#endif
         }
         m_end += m_reader( m_end, ( std::min )( buffer_free_after_end(), ( std::max )( amount - buffer_occupied(), Chunk ) ) );
      }

      template< rewind_mode M >
      [[nodiscard]] internal::rewind_guard< M, buffer_input > auto_rewind() noexcept
      {
         return internal::rewind_guard< M, buffer_input >( this );
      }

      [[nodiscard]] const frobnicator_t& rewind_save() noexcept
      {
         return m_current;
      }

      void rewind_restore( const frobnicator_t& data ) noexcept
      {
         m_current = data;
      }

      [[nodiscard]] tao::pegtl::position position( const frobnicator_t& it ) const
      {
         return tao::pegtl::position( it, m_source );
      }

      [[nodiscard]] tao::pegtl::position position() const
      {
         return position( m_current );
      }

      [[nodiscard]] const frobnicator_t& frobnicator() const noexcept
      {
         return m_current;
      }

      [[nodiscard]] std::size_t buffer_capacity() const noexcept
      {
         return m_maximum;
      }

      [[nodiscard]] std::size_t buffer_occupied() const noexcept
      {
         assert( m_end >= m_current.data );
         return static_cast< std::size_t >( m_end - m_current.data );
      }

      [[nodiscard]] std::size_t buffer_free_before_current() const noexcept
      {
         assert( m_current.data >= m_buffer.get() );
         return static_cast< std::size_t >( m_current.data - m_buffer.get() );
      }

      [[nodiscard]] std::size_t buffer_free_after_end() const noexcept
      {
         assert( m_buffer.get() + m_maximum >= m_end );
         return static_cast< std::size_t >( m_buffer.get() + m_maximum - m_end );
      }

   private:
      Reader m_reader;
      std::size_t m_maximum;
      std::unique_ptr< char[] > m_buffer;
      frobnicator_t m_current;
      char* m_end;
      const Source m_source;

   public:
      std::size_t private_depth = 0;
   };

}  // namespace tao::pegtl

#endif
