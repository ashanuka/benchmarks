/**
 * bmh.cpp - 
 * @author: Jonathan Beard
 * @version: Sat Oct 17 10:36:03 2015
 * 
 * Copyright 2015 Jonathan Beard
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <raft>
#include <raftio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <iterator>
#include <algorithm>
#ifdef GEM5
#include "gem5/m5ops.h"
#endif

template < class T > class search : public raft::kernel
{
public:
   search( const std::string && term ) : raft::kernel(),
                                         term_length( term.length() ),
                                         term( term )
   {
      input.addPort<  T >( "0" );
      output.addPort< std::size_t >( "0" );
   }
   
   search( const std::string &term ) : raft::kernel(),
                                       term_length( term.length() ),
                                       term( term )
   {
      input.addPort<  T >( "0" );
      output.addPort< std::size_t >( "0" );
   }

   virtual ~search() = default;

   virtual raft::kstatus run()
   {
      auto &chunk( input[ "0" ].template peek< T >() );
      auto it( chunk.begin() );
      do
      {
         it = std::search( it, chunk.end(),
                           term.begin(), term.end() );
         if( it != chunk.end() )
         {
            output[ "0" ].push( it.location() );
            it += 1;
         }
         else
         {
            break;
         }
      }
      while( true );
      input[ "0" ].unpeek();
      input[ "0" ].recycle( );
      return( raft::proceed );
   }
private:
   const std::size_t term_length;
   const std::string term;
};

int
main( int argc, char **argv )
{
    using chunk = raft::filechunk< 256 >;
    using fr    = raft::filereader< chunk, false >;
    using search = search< chunk >;
    using print = raft::print< std::size_t, '\n'>;
    
    if( argc < 4 )
    {
        std::cerr << "usage: ./wordsearch <file> <term> <#search workers>\n";
        exit( EXIT_FAILURE );
    }

    const std::string term( argv[ 2 ] );
    raft::map m;

    const auto kernel_count( atoi( argv[ 3 ] ) );
    fr   read( argv[ 1 ], (fr::offset_type) term.length(), kernel_count );

    print p( kernel_count );
    for( auto i( 0 ); i < kernel_count; i++ )
    {
        m += read[ std::to_string( i ) ] >> 
                raft::kernel::make< search >( term ) >> p[ std::to_string( i ) ];
    }

#ifdef GEM5
    m5_reset_stats(0, 0);
#endif

#ifdef VL
    m.exe< partition_dummy, pool_schedule, vlalloc, no_parallel >();
#else
    m.exe< partition_dummy, pool_schedule, stdalloc, no_parallel >();
#endif

#ifdef GEM5
    m5_dump_reset_stats(0, 0);
#endif
    return( EXIT_SUCCESS );
}
