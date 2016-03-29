/*! \file    CodeTree.hpp
 *  \brief   Builds the Huffman code tree.
 *  \author  Peter C. Chapin <PChapin@vtc.vsc.edu>
 */

#ifndef CODETREE_HPP
#define CODETREE_HPP

#include <string>

class CodeTree {
public:
    CodeTree( );

    void increment( int byte_value )
        { leaves[byte_value].count++; }

    void set_count( int byte_value, long count )
        { leaves[byte_value].count = count; }

    long get_count( int byte_value ) const
        { return leaves[byte_value].count; }

    long get_count_total( ) const;

    const std::string &get_code( int byte_value ) const
        { return codes[byte_value]; }

    void compute_probabilities( );
    void build_tree( );
    void build_codes( );
    void display_codes( );

private:
    // This type is used for the nodes of the Huffman code tree.
    struct Node {
        long  count;     // Number of counts for this node.
        Node *parent;    // Points at the node closer to the root.
        Node *more;      // Points at the child node with more counts.
        Node *less;      // Points at the child node with less counts.
        float P;         // Probability for this node. Only used with the leaves.
    };
    static int compare_table_entries(const void *e1, const void *e2);

    Node         leaves [256];
    Node        *sorted [256];
    Node        *actives[256];
    Node        *root;
    std::string  codes  [256];

    void find_smallest( int &smallest_index, int &next_smallest_index );

public:

    class Walker {
    public:
        Walker( const CodeTree &huffman ) : tree( &huffman ), current( huffman.root ) { }
        int code_finished( );
        void process_bit( int bit );
    private:
        const CodeTree *tree;
        const Node     *current;
    };

};

#endif

