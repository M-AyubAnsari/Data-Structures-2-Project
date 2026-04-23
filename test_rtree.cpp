#include "RTree.h"
#include <iostream>

int main() {
    RTree<double> tree;

    // Insert a few rectangles
    tree.insert({0, 0, 1, 1}, 1);
    tree.insert({2, 2, 3, 3}, 2);
    tree.insert({1, 1, 2, 2}, 3);
    tree.insert({4, 4, 5, 5}, 4);
    tree.insert({0, 3, 1, 4}, 5);
    tree.insert({3, 0, 4, 1}, 6);

    std::cout << "Count:  " << tree.count()  << "\n";
    std::cout << "Height: " << tree.height() << "\n\n";

    std::cout << "--- Tree structure ---\n";
    tree.print();

    // Range search: find everything that overlaps [0,0]-[2,2]
    std::cout << "\n--- Search [0,0]-[2,2] ---\n";
    auto hits = tree.search({0, 0, 2, 2});
    for (int id : hits)
        std::cout << "  hit id=" << id << "\n";

    // Remove an entry
    tree.remove({2, 2, 3, 3}, 2);
    std::cout << "\nAfter removing id 2, count = " << tree.count() << "\n";

    return 0;
}
