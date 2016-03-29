/*! \file    huff.cpp
    \brief   Huffman encoding based data compression.
    \author  Peter C. Chapin <PChapin@vtc.vsc.edu>

This file contains a program which analyizes and compresses files using huffman (ie
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

// Analyzes the previously opened file pointed at by infile.
static void analysis( CodeTree &huffman, FILE *infile )
{
    int  ch;                    // Holds data from file.
    long count     = 0L;        // Number of bytes from file.
    int  inner_count;           // Used to print progress report.

    printf( "\nAnalysis Phase...\n" );
    inner_count = 8 * K;
    while( ( ch = getc( infile ) ) != EOF ) {
        count++;
        huffman.increment( ch );
        if( --inner_count == 0 ) {
            printf( "\rHave read: %ldK", count/K );
            inner_count = 8*K;
        }
    }
    printf( "\rHave read: %ld bytes total.", count );

    huffman.compute_probabilities( );
}

// This function actually does the work of compression.
static void compress( const CodeTree &huffman, FILE *infile, char *out_name )
{
    spica::BitFile outfile;
    long     the_counts[256];
    int      ch;
    int      inner_count;
    long     count = 0;

    // Put the counts into a special array.
    for( int i = 0; i < 256; i++ )
        the_counts[i] = huffman.get_count( i );

    printf( "\n\n" );

    if( outfile.open_bit( out_name, spica::BitFileMode::Out ) == 0 ) {
        printf( "Cannot open the output file... aborting compression operation.\n" );
    }
    else {
        outfile.write_bitheader( the_counts, 256 * sizeof(long ));

        printf( "Compression Phase...\n" );
        inner_count = 8 * K;
        while( ( ch = getc( infile ) ) != EOF ) {
            for( const char &digit : huffman.get_code( ch ) )
                outfile.put_bit( ( digit == '0' ) ? 0 : 1 );
            count++;
            if( --inner_count == 0 ) {
                printf( "\rHave processed: %ldK", count / K );
                inner_count = 8 * K;
            }
        }
        printf( "\rHave processed: %ld total bytes of input.", count );
        outfile.close_bit( );
    }
}


/*----------------------------------*/
/*           Main Program           */
/*----------------------------------*/

int main( int argc, char *argv[] )
{
    FILE  *infile;
    int    return_value = 0;

    // Print header.
    printf( "HUFF  (Version 2.0a)  %s\n"
            "Public Domain Software by Peter Chapin\n", AdjDate( __DATE__ ) );

    // Make sure we've got files to work with.
    if( argc != 3 ) {
        printf( "\nError: Wrong number of arguments.\n\n"
                "USAGE: HUFF infile.bin outfile.bin\n" );
        return_value = 1;
    }
    
    // Make sure the file can be opened.
    else if( ( infile = fopen( argv[1], "rb" ) ) == NULL ) {
        printf( "\nError: Can't open %s for input.\n", argv[1] );
        return_value = 1;
    }

    // Increase the size of the input file's buffer.
    else if( setvbuf(infile, NULL, _IOFBF, 16 * K ) != 0 ) {
        printf( "\nError: Can't allocate memory for file buffers.\n" );
        fclose( infile );
        return_value = 1;
    }
    
    else {
        CodeTree huffman;

        // Analyize the file and print information on byte counts.
        analysis( huffman, infile );

        // Construct the Huffman code tree from the leaves.
        huffman.build_tree( );
        huffman.build_codes( );
        huffman.display_codes( );
    
        // Do the compression to the output file.
        rewind( infile );
        compress( huffman, infile, argv[2] );
        fclose( infile );
    }
    printf( "\n" );
    return return_value;
}
