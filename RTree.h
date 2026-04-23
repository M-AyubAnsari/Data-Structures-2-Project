#ifndef RTREE_H
#define RTREE_H

#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include <string>

template <typename T = double, int MAX_CHILD = 4, int MIN_CHILD = 2>
class RTree {
public:
    //minimum bounding rect
    struct Rect {
        T xMin, yMin;   // lower-left corner
        T xMax, yMax;   // upper-right corner

        Rect();
        Rect(T xMin, T yMin, T xMax, T yMax);

        /// Area of this rectangle.
        T area() const;

        /// Smallest rectangle that encloses both *this and other.
        Rect combine(const Rect& other) const;

        /// True if *this and other overlap.
        bool overlaps(const Rect& other) const;

        /// True if *this fully contains other.
        bool contains(const Rect& other) const;

        /// How much area would grow if we added other.
        T enlargement(const Rect& other) const;
    };

    //Node struct
    struct Node {
        bool  isLeaf;
        int   count;                  // number of active entries

        Rect  rects[MAX_CHILD];       // bounding rects of children / data
        Node* children[MAX_CHILD];    // child pointers  (internal nodes only)
        int   dataIds[MAX_CHILD];     // user data ids   (leaf nodes only)

        Node();
        ~Node();

        bool isFull() const;
        bool isUnderflow() const;

        /// Tight MBR covering all entries.
        Rect computeMBR() const;
    };

    RTree();
    ~RTree();

    // Insert a rectangle with an associated data id.
    void insert(const Rect& rect, int dataId);

    // Remove the first entry that matches rect and dataId.
    // Returns true if something was removed.
    bool remove(const Rect& rect, int dataId);

    /// Return all data ids whose MBR overlaps the query rectangle.
    std::vector<int> search(const Rect& queryRect) const;

    /// Total number of data entries in the tree.
    int  count() const;

    /// True if the tree is empty.
    bool empty() const;

    /// Remove everything.
    void clear();

    /// Height of the tree (0 if empty).
    int  height() const;

    /// Print the tree to stdout (for debugging).
    void print() const;

private:

    /// Split a full node + one overflow entry into two nodes.
    /// Returns the newly created sibling node.
    Node* splitNode(Node* node, const Rect& newRect,
                    Node* newChild, int newDataId);

    /// Recursive search-and-remove; collects orphans on underflow.
    bool  removeEntry(const Rect& rect, int dataId, Node* node,
                      std::vector<Rect>& orphanRects,
                      std::vector<int>&  orphanIds);

    /// Collect every leaf entry beneath node into the vectors.
    void  collectEntries(Node* node,
                         std::vector<Rect>& rects,
                         std::vector<int>&  ids);

    /// Recursive overlap search.
    void  searchImpl(const Node* node, const Rect& queryRect,
                     std::vector<int>& results) const;

    void  destroyTree(Node* node);
    int   heightImpl(const Node* node) const;
    int   countImpl(const Node* node) const;
    void  printImpl(const Node* node, int depth) const;

    Node* m_root;
    int   m_count;
};
#include "RTree.cpp"

#endif // RTREE_H