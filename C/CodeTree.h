/*! \file    CodeTree.h
 *  \brief   Builds the Huffman code tree.
 *  \author  Peter C. Chapin <PChapin@vtc.vsc.edu>
 */

#ifndef CODETREE_H
#define CODETREE_H

#include "str.h"

typedef struct CodeTreeNode {
    long   count;     // Number of counts for this node.
    double P;         // Probability for this node. Only used with the leaves.
    struct CodeTreeNode *parent;    // Points at the node closer to the root.
    struct CodeTreeNode *more;      // Points at the child node with more counts.
    struct CodeTreeNode *less;      // Points at the child node with less counts.
} CodeTreeNode;

typedef struct {
    CodeTreeNode  leaves [256];
    CodeTreeNode *sorted [256];
    CodeTreeNode *actives[256];
    CodeTreeNode *root;
    string codes[256];
} CodeTree;

void CodeTree_initialize( CodeTree *ct );
void CodeTree_increment( CodeTree *ct, int byte_value );
void CodeTree_set_count( CodeTree *ct, int byte_value, long count );
long CodeTree_get_count( const CodeTree *ct, int byte_value );
long CodeTree_get_count_total( const CodeTree *ct );
const string *CodeTree_get_code( const CodeTree *ct, int byte_value );
void CodeTree_compute_probabilities( CodeTree *ct );
void CodeTree_build_tree( CodeTree *ct );
void CodeTree_build_codes( CodeTree *ct );
void CodeTree_display_codes( const CodeTree *ct );

#endif
