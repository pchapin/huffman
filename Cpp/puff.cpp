/*! \file    puff.cpp
    \brief   Huffman encoding based data decompression.
    \author  Peter C. Chapin <chapinp@acm.org>

This file contains a program which analyizes and decompresses files using huffman (ie
statistically independent) data compression. It prints a number of interesting statistics about
the file.

LICENSE

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
*/

#include <cstdio>
#include <cstring>

#include "BitFile.hpp"
#include "CodeTree.hpp"

using namespace std;

#define K  1024

/*------------------------------------------*/
/*           Function Definitions           */
/*------------------------------------------*/

static char *AdjDate(const char *ANSI_Date)
  {
    static char  Buffer[13];
           char *Buffer_Pntr;

    strcpy(Buffer, ANSI_Date);
    for (Buffer_Pntr  = strchr(Buffer,'\0');
         Buffer_Pntr >= &Buffer[6];
         Buffer_Pntr--) {
      *(Buffer_Pntr+1) = *Buffer_Pntr;
    }
    Buffer[6] = ',';
    if (Buffer[4] == '0') {
      for (Buffer_Pntr = &Buffer[4]; *Buffer_Pntr; Buffer_Pntr++) {
        *Buffer_Pntr = *(Buffer_Pntr+1);
      }
    }
    return Buffer;
  }

static void analysis( CodeTree &huffman, spica::BitFile &infile )
{
  long counts[256];
  int  i;

  printf( "\nReading header from compressed file...\n" );
  infile.read_bitheader( counts, 256*sizeof( long ) );
  for( i = 0; i < 256; i++ ) {
      huffman.set_count( i, counts[i] );
  }
  huffman.compute_probabilities( );
}

// This function does the grunt work of decompressing.
static void decompress( const CodeTree &huffman, spica::BitFile &infile, char *out_name )
{
    FILE  *outfile;
    int    ch;
    int    byte_value;
    int    inner_count;
    long   count = 0;
    long   total_count = huffman.get_count_total( );

    printf( "\n\n" );

    // Open the output file.
    if( ( outfile = fopen( out_name, "wb" ) ) == NULL ) {
        printf( "Cannot open the output file... aborting decompression operation.\n" );
    }
    else {
        printf( "Decompression Phase...\n" );
        inner_count = 8 * K;
        CodeTree::Walker decoder( huffman );
        while( ( ch = infile.get_bit( ) ) != EOF ) {
            decoder.process_bit( ch );
            if( (byte_value = decoder.code_finished( )) != -1 ) {
                putc( byte_value, outfile );
                if( ++count == total_count ) break;
                if( --inner_count == 0 ) {
                    printf( "\rHave processed: %ldK", count / K );
                    inner_count = 8*K;
                }
            }
        }
        printf( "\rHave processed: %ld total bytes of output.", count );
        fclose( outfile );
    }
}


/*----------------------------------*/
/*           Main Program           */
/*----------------------------------*/

int main( int argc, char *argv[] )
{
    spica::BitFile  infile;
    int       return_value = 0;

    // Print header.
    printf( "PUFF  (Version 2.0a)  %s\n"
            "Public Domain Software by Peter Chapin\n", AdjDate( __DATE__ ) );

    // Make sure we've got files to work with.
    if( argc != 3 ) {
        printf( "\nError: Wrong number of arguments.\n\n"
                "USAGE: PUFF infile.bin outfile.bin\n" );
        return_value = 1;
    }

    // Make sure the file can be opened.
    else if( infile.open_bit( argv[1], spica::BitFileMode::In ) == 0 ) {
        printf( "\nError: Can't open %s for input.\n", argv[1] );
        return_value = 1;
    }

    else {
        CodeTree huffman;

        // Analyze the file and print information on byte counts.
        analysis( huffman, infile );

        // Construct the Huffman code tree from the leaves.
        huffman.build_tree( );
        huffman.build_codes( );
        huffman.display_codes( );

        // Do the decompression to the output file.
        decompress( huffman, infile, argv[2] );
        infile.close_bit( );
    }
    printf( "\n" );
    return return_value;
}
