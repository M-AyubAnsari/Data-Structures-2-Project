// ============================================================================
//  RTree.cpp  –  Implementation of the R-Tree methods declared in RTree.h
//
//  Algorithms based on:
//    A. Guttman, "R-Trees: A Dynamic Index Structure for Spatial Searching",
//    Proc. ACM SIGMOD, 1984.
//
//  This file is #include'd at the bottom of RTree.h because templates must
//  have their definitions visible to every translation unit that uses them.
// ============================================================================

#include <cmath>

// Shorthand so we don't repeat the long template prefix on every function.
#define TMPL  template <typename T, int MAX_CHILD, int MIN_CHILD>
#define RT    RTree<T, MAX_CHILD, MIN_CHILD>


TMPL
RT::Rect::Rect()
    : xMin(0), yMin(0), xMax(0), yMax(0) {}

TMPL
RT::Rect::Rect(T xMin, T yMin, T xMax, T yMax)
    : xMin(xMin), yMin(yMin), xMax(xMax), yMax(yMax) {}

TMPL
T RT::Rect::area() const {
    return (xMax - xMin) * (yMax - yMin);
}

TMPL
typename RT::Rect RT::Rect::combine(const Rect& other) const {
    return Rect(
        std::min(xMin, other.xMin),
        std::min(yMin, other.yMin),
        std::max(xMax, other.xMax),
        std::max(yMax, other.yMax)
    );
}

TMPL
bool RT::Rect::overlaps(const Rect& other) const {
    // Two rectangles overlap when they are not separated on any axis.
    if (xMin > other.xMax || xMax < other.xMin) return false;
    if (yMin > other.yMax || yMax < other.yMin) return false;
    return true;
}

TMPL
bool RT::Rect::contains(const Rect& other) const {
    return xMin <= other.xMin && yMin <= other.yMin &&
           xMax >= other.xMax && yMax >= other.yMax;
}

TMPL
T RT::Rect::enlargement(const Rect& other) const {
    return combine(other).area() - area();
}

TMPL
RT::Node::Node() : isLeaf(true), count(0) {
    for (int i = 0; i < MAX_CHILD; i++) {
        children[i] = nullptr;
        dataIds[i]  = -1;
    }
}

TMPL
RT::Node::~Node() {
    // Tree-wide cleanup is done by destroyTree().
}

TMPL
bool RT::Node::isFull() const {
    return count >= MAX_CHILD;
}

TMPL
bool RT::Node::isUnderflow() const {
    return count < MIN_CHILD;
}

TMPL
typename RT::Rect RT::Node::computeMBR() const {
    if (count == 0) return Rect();

    Rect mbr = rects[0];
    for (int i = 1; i < count; i++)
        mbr = mbr.combine(rects[i]);
    return mbr;
}

TMPL
RT::RTree() : m_root(new Node()), m_count(0) {}

TMPL
RT::~RTree() {
    destroyTree(m_root);
}

// ====================================================================
//  INSERT
//
//  1.  Walk from the root to the best leaf (choose the child whose MBR
//      needs the least enlargement to accommodate the new rectangle).
//  2.  If the leaf has room, add the entry.
//  3.  If the leaf is full, split it (quadratic split).
//  4.  Walk back up the path, updating MBRs and propagating splits.
//  5.  If the root itself was split, create a new root.
// ====================================================================

TMPL
void RT::insert(const Rect& rect, int dataId) {

    // ---- Step 1: walk down to the best leaf, recording the path ----
    std::vector<Node*> path;          // ancestors, root first
    Node* node = m_root;

    while (!node->isLeaf) {
        path.push_back(node);

        // Pick the child whose MBR needs the least enlargement.
        int   bestIdx = 0;
        T     bestEnl = node->rects[0].enlargement(rect);
        T     bestArea = node->rects[0].area();

        for (int i = 1; i < node->count; i++) {
            T enl  = node->rects[i].enlargement(rect);
            T area = node->rects[i].area();
            if (enl < bestEnl || (enl == bestEnl && area < bestArea)) {
                bestEnl  = enl;
                bestArea = area;
                bestIdx  = i;
            }
        }
        node = node->children[bestIdx];
    }

    // node is now the chosen leaf.

    // ---- Step 2 / 3: insert or split ----
    Node* sibling = nullptr;   // non-null if a split happened

    if (!node->isFull()) {
        // Room available – just add the entry.
        node->rects[node->count]   = rect;
        node->dataIds[node->count] = dataId;
        node->count++;
    } else {
        // Leaf is full – split it.
        sibling = splitNode(node, rect, nullptr, dataId);
    }

    // ---- Step 4: walk back up, fix MBRs, propagate splits ----
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        Node* parent = path[i];

        // Find the index of 'node' in its parent.
        int idx = -1;
        for (int j = 0; j < parent->count; j++) {
            if (parent->children[j] == node) { idx = j; break; }
        }

        // Update the MBR for the child we just modified.
        parent->rects[idx] = node->computeMBR();

        if (sibling != nullptr) {
            // We need to add 'sibling' to this parent.
            Rect sibMBR = sibling->computeMBR();

            if (!parent->isFull()) {
                parent->rects[parent->count]    = sibMBR;
                parent->children[parent->count] = sibling;
                parent->count++;
                sibling = nullptr;          // split absorbed
            } else {
                // Parent is also full – split the parent too.
                Node* parentSibling = splitNode(parent, sibMBR, sibling, -1);
                sibling = parentSibling;    // propagate upward
            }
        }

        node = parent;   // move one level up
    }

    // ---- Step 5: if the root was split, grow the tree by one level ----
    if (sibling != nullptr) {
        Node* newRoot    = new Node();
        newRoot->isLeaf  = false;

        newRoot->rects[0]    = m_root->computeMBR();
        newRoot->children[0] = m_root;

        newRoot->rects[1]    = sibling->computeMBR();
        newRoot->children[1] = sibling;

        newRoot->count = 2;
        m_root = newRoot;
    }

    m_count++;
}

// ====================================================================
//  SPLIT  (Quadratic Split – Guttman 1984, Section 3.5.2)
//
//  Given a full node (MAX_CHILD entries) plus one overflow entry,
//  distribute all MAX_CHILD+1 entries into the original node and a
//  new sibling, using the quadratic-cost algorithm:
//
//    1.  PickSeeds  – choose the pair whose combined MBR wastes the
//                     most area.
//    2.  PickNext   – repeatedly choose the unassigned entry with the
//                     greatest preference for one group over the other,
//                     and assign it to the preferred group.
//    3.  Stop when all entries are assigned, making sure each group
//        has at least MIN_CHILD entries.
// ====================================================================

TMPL
typename RT::Node* RT::splitNode(Node* node,
                                  const Rect& newRect,
                                  Node* newChild,
                                  int   newDataId) {
    const int TOTAL = MAX_CHILD + 1;

    // --- Gather all MAX_CHILD + 1 entries into temporary arrays ---
    Rect  tmpR[TOTAL];
    Node* tmpC[TOTAL];
    int   tmpD[TOTAL];

    for (int i = 0; i < MAX_CHILD; i++) {
        tmpR[i] = node->rects[i];
        tmpC[i] = node->children[i];
        tmpD[i] = node->dataIds[i];
    }
    tmpR[MAX_CHILD] = newRect;
    tmpC[MAX_CHILD] = newChild;
    tmpD[MAX_CHILD] = newDataId;

    // --- PickSeeds: find the pair with the most wasted area ---
    int seedA = 0, seedB = 1;
    T   worstWaste = std::numeric_limits<T>::lowest();

    for (int i = 0; i < TOTAL; i++) {
        for (int j = i + 1; j < TOTAL; j++) {
            T waste = tmpR[i].combine(tmpR[j]).area()
                    - tmpR[i].area() - tmpR[j].area();
            if (waste > worstWaste) {
                worstWaste = waste;
                seedA = i;
                seedB = j;
            }
        }
    }

    // --- Create the sibling and reset the original node ---
    Node* sibling    = new Node();
    sibling->isLeaf  = node->isLeaf;

    node->count    = 0;
    sibling->count = 0;

    // Assign the two seeds.
    auto assign = [&](Node* dst, int idx) {
        dst->rects[dst->count]    = tmpR[idx];
        dst->children[dst->count] = tmpC[idx];
        dst->dataIds[dst->count]  = tmpD[idx];
        dst->count++;
    };

    assign(node,    seedA);
    assign(sibling, seedB);

    bool used[TOTAL] = {};
    used[seedA] = true;
    used[seedB] = true;

    // --- Distribute the remaining entries ---
    int left = TOTAL - 2;   // entries still unassigned

    while (left > 0) {
        // If one group MUST take all remaining to reach MIN_CHILD, do so.
        if (node->count + left == MIN_CHILD) {
            for (int i = 0; i < TOTAL; i++)
                if (!used[i]) { assign(node, i); used[i] = true; }
            break;
        }
        if (sibling->count + left == MIN_CHILD) {
            for (int i = 0; i < TOTAL; i++)
                if (!used[i]) { assign(sibling, i); used[i] = true; }
            break;
        }

        // PickNext: find the entry with the greatest preference.
        Rect mbrA = node->computeMBR();
        Rect mbrB = sibling->computeMBR();

        int bestIdx  = -1;
        T   bestDiff = T(-1);
        int bestGroup = 0;          // 0 → node, 1 → sibling

        for (int i = 0; i < TOTAL; i++) {
            if (used[i]) continue;

            T d1 = mbrA.enlargement(tmpR[i]);
            T d2 = mbrB.enlargement(tmpR[i]);
            T diff = (d1 > d2) ? (d1 - d2) : (d2 - d1);

            if (bestIdx == -1 || diff > bestDiff) {
                bestDiff  = diff;
                bestIdx   = i;
                bestGroup = (d1 <= d2) ? 0 : 1;
            }
        }

        // Assign to the preferred group.
        if (bestGroup == 0)
            assign(node, bestIdx);
        else
            assign(sibling, bestIdx);

        used[bestIdx] = true;
        left--;
    }

    return sibling;
}

// ====================================================================
//  REMOVE
//
//  1.  Recursively find the leaf entry that matches (rect, dataId).
//  2.  Remove it.
//  3.  If a node underflows, collect its remaining entries as orphans,
//      delete the node, and remove it from the parent.
//  4.  After the recursion returns, re-insert all orphaned entries.
//  5.  If the root has only one child left, shrink the tree.
// ====================================================================

TMPL
bool RT::remove(const Rect& rect, int dataId) {
    std::vector<Rect> orphanRects;
    std::vector<int>  orphanIds;

    if (!removeEntry(rect, dataId, m_root, orphanRects, orphanIds))
        return false;       // not found

    m_count--;

    // Re-insert any orphaned entries.
    for (int i = 0; i < (int)orphanRects.size(); i++)
        insert(orphanRects[i], orphanIds[i]);

    // Shrink root if it is an internal node with a single child.
    while (!m_root->isLeaf && m_root->count == 1) {
        Node* oldRoot = m_root;
        m_root = m_root->children[0];
        oldRoot->children[0] = nullptr;   // prevent dangling delete
        delete oldRoot;
    }

    return true;
}

// ---- removeEntry (recursive) ----

TMPL
bool RT::removeEntry(const Rect& rect, int dataId, Node* node,
                     std::vector<Rect>& orphanRects,
                     std::vector<int>&  orphanIds) {

    if (node->isLeaf) {
        // Search this leaf for a matching entry.
        for (int i = 0; i < node->count; i++) {
            if (node->dataIds[i] == dataId && node->rects[i].overlaps(rect)) {
                // Remove by swapping with the last entry.
                node->count--;
                node->rects[i]   = node->rects[node->count];
                node->dataIds[i] = node->dataIds[node->count];
                return true;
            }
        }
        return false;   // not in this leaf
    }

    // Internal node – recurse into every child whose MBR overlaps rect.
    for (int i = 0; i < node->count; i++) {
        if (!node->rects[i].overlaps(rect))
            continue;

        if (removeEntry(rect, dataId, node->children[i],
                        orphanRects, orphanIds)) {
            Node* child = node->children[i];

            if (child->isLeaf && child->count == 0) {
                // Child is now empty – delete it and remove from parent.
                delete child;
                node->count--;
                node->rects[i]    = node->rects[node->count];
                node->children[i] = node->children[node->count];
            }
            else if (child->isUnderflow()) {
                // Collect all leaf entries from this subtree as orphans.
                collectEntries(child, orphanRects, orphanIds);
                destroyTree(child);
                // Remove the child slot from the parent.
                node->count--;
                node->rects[i]    = node->rects[node->count];
                node->children[i] = node->children[node->count];
            }
            else {
                // Just tighten the MBR.
                node->rects[i] = child->computeMBR();
            }

            return true;
        }
    }
    return false;
}

// ---- collectEntries ----

TMPL
void RT::collectEntries(Node* node,
                        std::vector<Rect>& rects,
                        std::vector<int>&  ids) {
    if (node->isLeaf) {
        for (int i = 0; i < node->count; i++) {
            rects.push_back(node->rects[i]);
            ids.push_back(node->dataIds[i]);
        }
    } else {
        for (int i = 0; i < node->count; i++)
            collectEntries(node->children[i], rects, ids);
    }
}

// ====================================================================
//  SEARCH  –  overlap query
//
//  Starting from the root, visit every node whose MBR overlaps the
//  query rectangle.  At leaf level, report matching data ids.
// ====================================================================

TMPL
std::vector<int> RT::search(const Rect& queryRect) const {
    std::vector<int> results;
    searchImpl(m_root, queryRect, results);
    return results;
}

TMPL
void RT::searchImpl(const Node* node, const Rect& queryRect,
                    std::vector<int>& results) const {
    if (node->isLeaf) {
        for (int i = 0; i < node->count; i++) {
            if (node->rects[i].overlaps(queryRect))
                results.push_back(node->dataIds[i]);
        }
    } else {
        for (int i = 0; i < node->count; i++) {
            if (node->rects[i].overlaps(queryRect))
                searchImpl(node->children[i], queryRect, results);
        }
    }
}

// ====================================================================
//  UTILITIES
// ====================================================================

TMPL int  RT::count()  const { return m_count; }
TMPL bool RT::empty()  const { return m_count == 0; }
TMPL int  RT::height() const { return heightImpl(m_root); }

TMPL
void RT::clear() {
    destroyTree(m_root);
    m_root  = new Node();
    m_count = 0;
}

TMPL
void RT::print() const {
    if (m_count == 0) {
        std::cout << "(empty R-tree)\n";
        return;
    }
    printImpl(m_root, 0);
}

// ---- private helpers ----

TMPL
void RT::destroyTree(Node* node) {
    if (node == nullptr) return;
    if (!node->isLeaf) {
        for (int i = 0; i < node->count; i++)
            destroyTree(node->children[i]);
    }
    delete node;
}

TMPL
int RT::heightImpl(const Node* node) const {
    if (node == nullptr)  return 0;
    if (node->isLeaf)     return 1;
    return 1 + heightImpl(node->children[0]);
}

TMPL
int RT::countImpl(const Node* node) const {
    if (node == nullptr) return 0;
    if (node->isLeaf)    return node->count;

    int total = 0;
    for (int i = 0; i < node->count; i++)
        total += countImpl(node->children[i]);
    return total;
}

TMPL
void RT::printImpl(const Node* node, int depth) const {
    std::string indent(depth * 2, ' ');

    if (node->isLeaf) {
        for (int i = 0; i < node->count; i++) {
            std::cout << indent << "DATA id=" << node->dataIds[i]
                      << "  [" << node->rects[i].xMin
                      << ", "  << node->rects[i].yMin
                      << ", "  << node->rects[i].xMax
                      << ", "  << node->rects[i].yMax << "]\n";
        }
    } else {
        for (int i = 0; i < node->count; i++) {
            std::cout << indent << "NODE  ["
                      << node->rects[i].xMin << ", "
                      << node->rects[i].yMin << ", "
                      << node->rects[i].xMax << ", "
                      << node->rects[i].yMax << "]\n";
            printImpl(node->children[i], depth + 1);
        }
    }
}

#undef TMPL
#undef RT
