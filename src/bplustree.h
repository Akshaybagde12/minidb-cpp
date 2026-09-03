#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace minidb {

class BPlusTree {
public:
    explicit BPlusTree(std::size_t max_keys = 15);
    ~BPlusTree();

    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    void insert(int key, std::uint64_t offset);
    std::optional<std::uint64_t> search(int key) const;
    bool erase(int key); // lazy leaf deletion; no merge/rebalance
    void clear();

private:
    struct Node {
        explicit Node(bool leaf) : is_leaf(leaf) {}
        bool is_leaf;
        std::vector<int> keys;
        std::vector<Node*> children;
        std::vector<std::uint64_t> offsets;
        Node* next = nullptr;
    };

    Node* root_ = nullptr;
    std::size_t max_keys_;

    Node* find_leaf(int key) const;
    Node* find_parent(Node* current, Node* child) const;
    void insert_into_parent(Node* left, int separator, Node* right);
    void split_leaf(Node* leaf);
    void split_internal(Node* node);
    void destroy(Node* node);
};

} // namespace minidb
