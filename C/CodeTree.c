/*! \file    CodeTree.c
 *  \brief   Builds the Huffman code tree.
 *  \author  Peter C. Chapin <PChapin@vtc.vsc.edu>
 */

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CodeTree.h"

// ===========================
// Internally Linked Functions
// ===========================

// Used when sorting the leaves. The sort is based on the counts associated with each character.
static int compare_table_entries( const void *e1, const void *e2 )
{
    int return_value;

    if( (*(CodeTreeNode **)e1)->count > (*(CodeTreeNode **)e2)->count ) {
        return_value = -1;
    }
    else if( (*(CodeTreeNode **)e1)->count == (*(CodeTreeNode **)e2)->count ) {
        return_value =  0;
    }
    else {
        return_value = +1;
    }
    return return_value;
}


static void find_smallest( CodeTree *ct, int *smallest_index, int *next_smallest_index )
{
    long smallest_value      = LONG_MAX;
    long next_smallest_value = LONG_MAX;

    // Loop over all the elements in the array.
    for( int index = 0; index < 256; index++ ) {
        
        // If this element is pointing at nothing, ignore it.
        if( ct->actives[index] == NULL ) continue;

        // If this element is smaller than anything we've seen so far...
        if( ct->actives[index]->count < smallest_value ) {
            next_smallest_value = smallest_value;
            *next_smallest_index = *smallest_index;
            smallest_value = ct->actives[index]->count;
            *smallest_index = index;
        }

        // If this element is smaller than the old next to smallest thing...
        else if( ct->actives[index]->count < next_smallest_value ) {
            next_smallest_value = ct->actives[index]->count;
            *next_smallest_index = index;
        }
    }
}

//=================
// CodeTree Methods
//=================

void CodeTree_initialize( CodeTree *ct )
{
    for( int i = 0; i < 256; i++ ) {
        ct->leaves[i].count  = 0L;
        ct->leaves[i].parent = NULL;
        ct->leaves[i].more = NULL;
        ct->leaves[i].less = NULL;
        ct->leaves[i].P = 0.0;
        ct->sorted[i] = &ct->leaves[i];
        ct->actives[i] = &ct->leaves[i];
        string_init( &ct->codes[i] );
    }
    ct->root = NULL;
}


void CodeTree_increment( CodeTree *ct, int byte_value )
{
    ct->leaves[byte_value].count++;
}

void CodeTree_set_count( CodeTree *ct, int byte_value, long count )
{
    ct->leaves[byte_value].count = count;
}

long CodeTree_get_count( const CodeTree *ct, int byte_value )
{
    return ct->leaves[byte_value].count;
}

long CodeTree_get_count_total( const CodeTree *ct )
{
    long count = 0;
    for( int i = 0; i < 256; ++i ) {
        count += ct->leaves[i].count;
    }
    return count;
}

const string *CodeTree_get_code( const CodeTree *ct, int byte_value )
{
    return &ct->codes[byte_value];
}

void CodeTree_compute_probabilities( CodeTree *ct )
{
    long   count;
    double entropy     = 0.0;         // See reference.
    int    number_used = 0;           // Number of codes used.
    const double log_2 = log( 2.0 );  // Precalculated.
    int   i;                         // Loop index.

    printf( "\n\nComputing probablities..." );
    count = CodeTree_get_count_total( ct );
    for( i = 0; i < 256; i++ ) {
        ct->leaves[i].P = (double)ct->leaves[i].count / count;
        if( ct->leaves[i].count != 0 ) {
            entropy += -ct->leaves[i].P * log( ct->leaves[i].P ) / log_2;
            number_used++;
        }
    }

    printf( "\nSorting probablities..." );
    qsort( ct->sorted, 256, sizeof(struct CodeTreeNode *), compare_table_entries );

    printf( "\n\n" );
    printf( "Five most frequent bytes:\n" );
    for( i = 0; i < 5; i++ ) {
        printf( "%d:  %02Xh, %5.2f%%,  ",
                i+1, (int)( ct->sorted[i] - ct->leaves ), ct->leaves[(int)( ct->sorted[i] - ct->leaves )].P * 100 );
        if( iscntrl( (int)( ct->sorted[i] - ct->leaves ) ) )
            printf( "\'^%c\'\n", (int)( ct->sorted[i] - ct->leaves ) + 0x40 );
        else
            printf( "\'%c\'\n", (int)( ct->sorted[i] - ct->leaves ) );
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


void CodeTree_build_tree( CodeTree *ct )
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
        find_smallest( ct, &smallest, &next_smallest );

        // Build a new node and initialize its fields.
        struct CodeTreeNode *new_node =
            (struct CodeTreeNode *)malloc( sizeof(struct CodeTreeNode) );
        ct->root = new_node;
        new_node->count  = ct->actives[smallest]->count + ct->actives[next_smallest]->count;
        new_node->parent = NULL;
        new_node->P      = 0.0;

        // If the counts are not equal, point less at the node with the smaller count. The
        // complexity of these steps insures that this program generates compressed files
        // according to a strict specification. This is for compatibility with student written
        // code.
        //
        if( ct->actives[smallest]->count != ct->actives[next_smallest]->count ) {
            new_node->less = ct->actives[smallest];
            new_node->more = ct->actives[next_smallest];
        }

        // If they are equal, point less at the node reached using the smaller index in actives.
        // It *may* not be true that smallest is a smaller index than next_smallest -- even if
        // the counts are equal. That will depend on how Find_Smallest() works. The code below
        // makes no assumptions.
        //
        else {
            new_node->less = ct->actives[smallest < next_smallest ? smallest : next_smallest];
            new_node->more = ct->actives[smallest < next_smallest ? next_smallest : smallest];
        }

        // Adjust the actives array. We have to be sure to put the non-NULL pointer into the
        // array at the smallest index.
        //
        ct->actives[smallest]->parent      = new_node;
        ct->actives[next_smallest]->parent = new_node;
        if( smallest < next_smallest ) {
            ct->actives[smallest]      = new_node;
            ct->actives[next_smallest] = NULL;
        }
        else {
            ct->actives[next_smallest] = new_node;
            ct->actives[smallest]      = NULL;
        }

        // Keep track of the count.
        item_count--;
    }

    printf( "\nDone! ENTER to continue..." );
    while( getchar() != '\n' ) /* Null */ ;
}


void CodeTree_build_codes( CodeTree *ct )
{
    struct CodeTreeNode *current;

    printf( "\nAssigning new bit patterns for the bytes in the file...\n" );

    // Generate codes for every leaf.
    for( int index = 0; index < 256; index++ ) {

        // Start with this leaf and work towards the root.
        current = &ct->leaves[index];
        while( current->parent != NULL ) {

            // Remember the address of this node before we move.
            struct CodeTreeNode *temp = current;

            // Advance to the parent node and see if the old node was more or less.
            current = current->parent;
            if( current->more == temp ) string_appendchar( &ct->codes[index], '1' );
            if( current->less == temp ) string_appendchar( &ct->codes[index], '0' );
        }

        // Reverse the characters in this code.
        string_reverse( &ct->codes[index] );
    }

    printf( "\nDone! ENTER to continue..." );
    while( getchar( ) != '\n' ) /* Null */ ;
}


void CodeTree_display_codes( const CodeTree *ct )
{
    printf( "\nCalculated Huffman codes for bytes with nonzero counts...\n\n" );
    
    for( int index = 0; index < 256; index++ ) {
        if( ct->leaves[index].count == 0 ) continue;

        if( isgraph( index ) )
            printf( "'%c' ", index );
        else
            printf( "    " );

        // string_getcharp wants a pointer to a non-const string because it returns a pointer to
        // a non-const character. In this case we don't modify the string via the char* returned
        // so it is safe to cast away const.
        printf( "%02X: (%6ld) %s\n", index, ct->leaves[index].count, string_getcharp( (string *)&ct->codes[index] ) );
    }
    
    printf( "\nENTER to continue..." );
    while( getchar( ) != '\n' ) /* Null */ ;
}
