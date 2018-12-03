/*! \file    puff.c
 *  \brief   Huffman encoding based data decompression.
 *  \author  Peter C. Chapin <chapinp@acm.org>
 *
 * This file contains a program which analyizes and compresses files using huffman (ie
 * statistically independent) data compression. It prints a number of interesting statistics
 * about the file.
 *
 * LICENSE
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA
*/

#include <stdio.h>
#include <string.h>

#include "BitFile.h"
#include "CodeTree.h"

#define K  1024

/*------------------------------------------*/
/*           Function Definitions           */
/*------------------------------------------*/

static char *adjust_date(const char *ANSI_Date)
{
    static char  buffer[13];
           char *buffer_ptr;

    strcpy(buffer, ANSI_Date);
    for( buffer_ptr  = strchr(buffer,'\0'); buffer_ptr >= &buffer[6]; buffer_ptr-- ) {
        *( buffer_ptr + 1 ) = *buffer_ptr;
    }
    buffer[6] = ',';
    if( buffer[4] == '0' ) {
        for( buffer_ptr = &buffer[4]; *buffer_ptr; buffer_ptr++ ) {
            *buffer_ptr = *( buffer_ptr + 1 );
        }
    }
    return buffer;
}

static void analysis( CodeTree *huffman, BitFile *infile )
{
  long counts[256];
  int  i;

  printf( "\nReading header from compressed file...\n" );
  read_bitheader( infile, counts, 256*sizeof( long ) );
  for( i = 0; i < 256; i++ ) {
      set_count( huffman, i, counts[i] );
  }
  compute_probabilities( huffman );
}

// This function does the grunt work of decompressing.
static void decompress( const CodeTree *huffman, BitFile *infile, char *out_name )
{
    FILE  *outfile;
    int    ch;
    int    byte_value;
    int    inner_count;
    long   count = 0;
    long   total_count = get_count_total( huffman );

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
    BitFile  infile;
    int      return_value = 0;

    // Print header.
    printf( "PUFF  (Version 2.0a)  %s\n"
            "Public Domain Software by Peter Chapin\n", adjust_date( __DATE__ ) );

    // Make sure we've got files to work with.
    if( argc != 3 ) {
        printf( "\nError: Wrong number of arguments.\n\n"
                "USAGE: PUFF infile.bin outfile.bin\n" );
        return_value = 1;
    }

    // Make sure the file can be opened.
    else if( open_bit( &infile, argv[1], Bit_In ) == 0 ) {
        printf( "\nError: Can't open %s for input.\n", argv[1] );
        return_value = 1;
    }

    else {
        CodeTree huffman;

        // Analyze the file and print information on byte counts.
        analysis( &huffman, &infile );

        // Construct the Huffman code tree from the leaves.
        build_tree( &huffman );
        build_codes( &huffman );
        display_codes( &huffman );

        // Do the decompression to the output file.
        decompress( &huffman, infile, argv[2] );
        close_bit( &infile );
    }
    printf( "\n" );
    return return_value;
}
