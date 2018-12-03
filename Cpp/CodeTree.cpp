/*! \file    CodeTree.cpp
 *  \brief   Builds the Huffman code tree.
 *  \author  Peter C. Chapin <chapinp@acm.org>
 */

#include <algorithm>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CodeTree.hpp"

using namespace std;

// Used when sorting the leaves. The sort is based on the counts associated with each character.
int CodeTree::compare_table_entries(const void *e1, const void *e2)
{
    int return_value;

    if( (*(Node **)e1)->count > (*(Node **)e2)->count ) {
        return_value = -1;
    }
    else if( (*(Node **)e1)->count == (*(Node **)e2)->count ) {
        return_value =  0;
    }
    else return_value = +1;
    return return_value;
}


CodeTree::CodeTree( )
{
    for( int i = 0; i < 256; i++ ) {
        leaves[i].count  = 0L;
        leaves[i].parent = nullptr;
        leaves[i].more   = nullptr;
        leaves[i].less   = nullptr;
        leaves[i].P      = 0.0;
        sorted[i]        = &leaves[i];
        actives[i]       = &leaves[i];
    }
}


long CodeTree::get_count_total( ) const
{
    long count = 0;
    for( const auto &leaf : leaves ) {
        count += leaf.count;
    }
    return count;
}


void CodeTree::compute_probabilities( )
{
    long  count;
    float entropy     = 0.0;         // See reference.
    int   number_used = 0;           // Number of codes used.
    const float log_2 = log( 2.0 );  // Precalculated.
    int   i;                         // Loop index.

    printf( "\n\nComputing probablities..." );
    count = get_count_total( );
    for( i = 0; i < 256; i++ ) {
        leaves[i].P = (float)leaves[i].count / count;
        if( leaves[i].count != 0 ) {
            entropy += -leaves[i].P * log( leaves[i].P ) / log_2;
            number_used++;
        }
    }

    printf( "\nSorting probablities..." );
    qsort( sorted, 256, sizeof(struct Node *), compare_table_entries );

    printf( "\n\n" );
    printf( "Five most frequent bytes:\n" );
    for( i = 0; i < 5; i++ ) {
        printf( "%d:  %02Xh, %5.2f%%,  ",
                i+1, (int)( sorted[i] - leaves ), leaves[(int)( sorted[i] - leaves )].P * 100 );
        if( iscntrl( (int)( sorted[i] - leaves ) ) )
            printf( "\'^%c\'\n", (int)( sorted[i] - leaves ) + 0x40 );
        else
            printf( "\'%c\'\n", (int)( sorted[i] - leaves ) );
    }
    printf( "Number of different bytes not used = %d\n", 256 - number_used );

    printf( "\n" );
    printf( "Entropy           = %4.2f  (Average bits/byte of information)\n", entropy );
    printf( "Redundancy        = %4.2f  (Average bits/byte of excess)\n", 8.0 - entropy );
    printf( "Redundancy        = %4.1f%% (Percentage of file which is redundant)\n", (1.0 - entropy/8.0) * 100.0 );
    printf( "Compression Ratio = %4.2f  (If all redundancy removed)\n", 8.0 / entropy );
    printf( "File Sizes (Assuming 100%% compression efficiency and no overhead):\n" );
    printf( "  Before = %ld\n", count );
    printf( "  After  = %ld\n", (long) ((float)count * entropy / 8.0) );

    printf( "\nENTER to continue..." );
    while( getchar() != '\n' ) /* Null */ ;
}


void CodeTree::build_tree( )
{
    int item_count = 256;
    int smallest;
    int next_smallest;

    printf( "\nConstructing the Huffman code tree...\n" );

    // Keep working until there is only one active node left (ie the root).
    while( item_count > 1 ) {

        // Find the indicies into the actives array that refer to the nodes with the smallest
        // counts.
        //
        find_smallest( smallest, next_smallest );

        // Build a new node and initialize its fields.
        struct Node *new_node = new Node;
        root = new_node;
        new_node->count  = actives[smallest]->count + actives[next_smallest]->count;
        new_node->parent = nullptr;
        new_node->P      = 0.0;

        // If the counts are not equal, point less at the node with the smaller count. The
        // complexity of these steps insures that this program generates compressed files
        // according to a strict specification. This is for compatibility with student written
        // code.
        //
        if( actives[smallest]->count != actives[next_smallest]->count ) {
            new_node->less = actives[smallest];
            new_node->more = actives[next_smallest];
        }

        // If they are equal, point less at the node reached using the smaller index in actives.
        // It *may* not be true that smallest is a smaller index than next_smallest -- even if
        // the counts are equal. That will depend on how Find_Smallest() works. The code below
        // makes no assumptions.
        //
        else {
            new_node->less = actives[smallest < next_smallest ? smallest : next_smallest];
            new_node->more = actives[smallest < next_smallest ? next_smallest : smallest];
        }

        // Adjust the actives array. We have to be sure to put the non-NULL pointer into the
        // array at the smallest index.
        //
        actives[smallest]->parent      = new_node;
        actives[next_smallest]->parent = new_node;
        if( smallest < next_smallest ) {
            actives[smallest]      = new_node;
            actives[next_smallest] = nullptr;
        }
        else {
            actives[next_smallest] = new_node;
            actives[smallest]      = nullptr;
        }

        // Keep track of the count.
        item_count--;
    }

    printf( "\nDone! ENTER to continue..." );
    while( getchar() != '\n' ) /* Null */ ;
}


void CodeTree::build_codes( )
{
    struct Node *current;

    printf( "\nAssigning new bit patterns for the bytes in the file...\n" );

    // Generate codes for every leaf.
    for( int index = 0; index < 256; index++ ) {

        // Start with this leaf and work towards the root.
        current = &leaves[index];
        while( current->parent != nullptr ) {

            // Remember the address of this node before we move.
            struct Node *temp = current;

            // Advance to the parent node and see if the old node was more or less.
            current = current->parent;
            if( current->more == temp ) codes[index].append( 1, '1' );
            if( current->less == temp ) codes[index].append( 1, '0' );
        }

        // Reverse the characters in this code.
        reverse( codes[index].begin( ), codes[index].end( ) );
    }

    printf( "\nDone! ENTER to continue..." );
    while( getchar( ) != '\n' ) /* Null */ ;
}


void CodeTree::display_codes( )
{
    printf( "\nCalculated Huffman codes for bytes with nonzero counts...\n\n" );
    
    for( int index = 0; index < 256; index++ ) {
        if( leaves[index].count == 0 ) continue;

        if( isgraph( index ) ) printf( "'%c' ", index );
        else printf( "    " );
        printf( "%02X: (%6ld) %s\n", index, leaves[index].count, codes[index].c_str( ) );
    }
    
    printf( "\nENTER to continue..." );
    while( getchar( ) != '\n' ) /* Null */ ;
}


void CodeTree::find_smallest( int &smallest_index, int &next_smallest_index )
{
    long smallest_value      = LONG_MAX;
    long next_smallest_value = LONG_MAX;

    // Loop over all the elements in the array.
    for( int index = 0; index < 256; index++ ) {
        
        // If this element is pointing at nothing, ignore it.
        if( actives[index] == nullptr ) continue;

        // If this element is smaller than anything we've seen so far...
        if( actives[index]->count < smallest_value ) {
            next_smallest_value = smallest_value;
            next_smallest_index = smallest_index;
            smallest_value      = actives[index]->count;
            smallest_index      = index;
        }

        // If this element is smaller than the old next to smallest thing...
        else if( actives[index]->count < next_smallest_value ) {
            next_smallest_value = actives[index]->count;
            next_smallest_index = index;
        }
    }
}


int CodeTree::Walker::code_finished( )
{
    int byte_value;

    if( current->more != nullptr) return -1;

    byte_value = current - tree->leaves;
    current = tree->root;
    return byte_value;
}


void CodeTree::Walker::process_bit( int bit_value )
{
    if( bit_value == 1 ) current = current->more;
    if( bit_value == 0 ) current = current->less;
}
