#include "prev.h"

#include <limits>
#include <memory>
#include <vector>

// In this task, I use a persistent segment tree.

struct Node {
    int lo, hi;
    int maxj; // the largest j such that x_j ∈ [lo, hi]
    std::shared_ptr<Node> left, right;

    // index - the index of the value in seq
    // low, high - the minimum and maximum value of the range
    Node(int index, int low, int high)
        : lo(low), hi(high), maxj(index), left(nullptr), right(nullptr) {}
};

typedef std::shared_ptr<Node> Segment_tree;

// Vector of pointers to roots
// The index i in the vector seq corresponds to the root in trees[i+1]
// In i = 0, we hold the root covering the interval [INT_MIN, INT_MAX] and pointing to nullptr
std::vector<Segment_tree> trees;

// --------------------------- Helper functions -------------------------------

// Checks if a given node is a singleton
bool isSingleton(const Node &node) {
    return node.hi == node.lo;
}

// Computes the mean in such a way as to avoid overflow
int mean(const int a, const int b) {
    if((a > 0 && b > 0) || (a < 0 && b < 0)) {
        return (a - b) / 2 + b;
    }
    else {
        return (a + b) / 2;
    }
}

// ----------------------- Functions implementing the task --------------------------

// Complexity: T(n) = O(seq.size()*log z), z -> range of int
void init(const std::vector<int> &seq) {
    // Create a full node [INT_MIN, INT_MAX]
    Segment_tree fullTree = std::make_shared<Node>(-1, std::numeric_limits<int>::min(),
                                                   std::numeric_limits<int>::max());
    trees.push_back(fullTree);
    for (int i : seq) {
        pushBack(i);
    }
}

// Complexity: T(n) = O(log z), z -> range of int
int prevInRangeRec(int lo, int hi, const std::shared_ptr<Node>& node) {
    // If the given node is nullptr, return -1
    if (node == nullptr) {
        return -1;
    }

    // If the given range is contained within the searched range
    if (lo <= node -> lo && hi >= node -> hi) {
        return node -> maxj;
    }

    // If the given range has an empty intersection with the searched range
    if (hi < node -> lo || lo > node -> hi) {
        return -1;
    }

    // If none of the above situations occur
    return std::max(prevInRangeRec(lo, hi, node -> left),
                    prevInRangeRec(lo, hi, node -> right));
}

int prevInRange(int i, int lo, int hi) {
    return prevInRangeRec(lo, hi, trees[i+1]);
}

// Complexity: T(n) = O(log z), z -> range of int
void pushBack(int Value) {
    // index - index of the given value
    int index = (int)trees.size() - 1;

    // Initialize the root for the new value
    Segment_tree newRoot = std::make_shared<Node>(*trees.back());
    newRoot -> maxj = index;
    trees.push_back(newRoot);

    // curr - to traverse the tree
    Segment_tree curr = newRoot;

    while(!isSingleton(*curr)) {
        // m - average of lo and hi
        int m = mean(curr -> hi, curr -> lo);

        // Below the average goes to the left child
        if (Value <= m) {
            Segment_tree newNode;
            // If in the previous version of the tree left was nullptr, create a new left child
            if (curr -> left == nullptr) {
                newNode = std::make_shared<Node>(index, curr -> lo, m);
            }
            // If in the previous version of the tree left was not nullptr, just copy the Node
            // and change its maxj
            else {
                newNode = std::make_shared<Node>(*curr->left);
                newNode -> maxj = index;
            }
            curr -> left = newNode;
            curr = newNode;
        }
        // Above the average goes to the right child; the code is analogous to the left
        else {
            Segment_tree newNode;
            if (curr -> right == nullptr) {
                newNode = std::make_shared<Node>(index, m + 1, curr -> hi);
            }
            else {
                newNode = std::make_shared<Node>(*curr->right);
                newNode -> maxj = index;
            }
            curr -> right = newNode;
            curr = newNode;
        }
    }
}

void done() {
    // Clear trees
    // Since we worked with std::shared_ptr, the rest of the cleanup happens automatically
    trees.clear();
}
