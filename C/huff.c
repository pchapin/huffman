/*! \file    huff.c
 *  \brief   Huffman encoding based data compression.
 *  \author  Peter C. Chapin <PChapin@vtc.vsc.edu>
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

// Analyzes the previously opened file pointed at by infile.
static void analysis( CodeTree *huffman, FILE *infile )
{
    int  ch;                    // Holds data from file.
    long count     = 0L;        // Number of bytes from file.
    int  inner_count;           // Used to print progress report.

    printf( "\nAnalysis Phase...\n" );
    inner_count = 8 * K * K;
    while( ( ch = getc( infile ) ) != EOF ) {
        count++;
        CodeTree_increment( huffman, ch );
        if( --inner_count == 0 ) {
            printf( "\rHave read: %ld MiB", count /(K*K) );
            inner_count = 8 * K * K;
        }
    }
    printf( "\rHave read: %ld bytes total.", count );

    CodeTree_compute_probabilities( huffman );
}

// This function actually does the work of compression.
static void compress( const CodeTree *huffman, FILE *infile, char *out_name )
{
    BitFile outfile;
    long    the_counts[256];
    int     ch;
    int     inner_count;
    long    count = 0;

    // Put the counts into a special array.
    for( int i = 0; i < 256; i++ )
        the_counts[i] = CodeTree_get_count( huffman, i );

    printf( "\n\n" );

    if( open_bit( &outfile, out_name, Bit_Out ) == 0 ) {
        printf( "Cannot open the output file... aborting compression operation.\n" );
    }
    else {
        write_bitheader( &outfile, the_counts, 256 * sizeof(long) );

        printf( "Compression Phase...\n" );
        inner_count = 8 * K;
        while( ( ch = getc( infile ) ) != EOF ) {
            const string *code = CodeTree_get_code( huffman, ch );
            size_t code_length = string_length( code );
            for( int i = 0; i < code_length; ++i ) {
                char digit = string_getcharat( code, i );
                put_bit( &outfile, ( digit == '0' ) ? 0 : 1 );
            }
            count++;
            if( --inner_count == 0 ) {
                printf( "\rHave processed: %ldK", count / K );
                inner_count = 8 * K;
            }
        }
        printf( "\rHave processed: %ld total bytes of input.", count );
        close_bit( &outfile );
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
    printf( "Huff (Version 2.0b) %s\n"
            "Copyright 2015 by Peter Chapin\n"
            "    \"I'll huff and I'll puff and I'll...\"\n", adjust_date( __DATE__ ) );

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
        CodeTree_initialize( &huffman );

        // Analyize the file and print information on byte counts.
        analysis( &huffman, infile );

        // Construct the Huffman code tree from the leaves.
        CodeTree_build_tree( &huffman );
        CodeTree_build_codes( &huffman );
        CodeTree_display_codes( &huffman );
    
        // Do the compression to the output file.
        rewind( infile );
        compress( &huffman, infile, argv[2] );
        fclose( infile );
    }
    printf( "\n" );
    return return_value;
}
