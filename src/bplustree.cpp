#include "bplustree.h"

#include <algorithm>
#include <stdexcept>

namespace minidb {

BPlusTree::BPlusTree(std::size_t max_keys) : max_keys_(max_keys) {
    if (max_keys_ < 3) {
        throw std::invalid_argument("B+ tree max_keys must be at least 3");
    }
}

BPlusTree::~BPlusTree() { clear(); }

void BPlusTree::destroy(Node* node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (Node* child : node->children) destroy(child);
    }
    delete node;
}

void BPlusTree::clear() {
    destroy(root_);
    root_ = nullptr;
}

BPlusTree::Node* BPlusTree::find_leaf(int key) const {
    Node* current = root_;
    while (current && !current->is_leaf) {
        auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
        const auto idx = static_cast<std::size_t>(std::distance(current->keys.begin(), it));
        current = current->children.at(idx);
    }
    return current;
}

std::optional<std::uint64_t> BPlusTree::search(int key) const {
    Node* leaf = find_leaf(key);
    if (!leaf) return std::nullopt;
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    if (it == leaf->keys.end() || *it != key) return std::nullopt;
    const auto idx = static_cast<std::size_t>(std::distance(leaf->keys.begin(), it));
    return leaf->offsets[idx];
}

void BPlusTree::insert(int key, std::uint64_t offset) {
    if (!root_) {
        root_ = new Node(true);
        root_->keys.push_back(key);
        root_->offsets.push_back(offset);
        return;
    }

    Node* leaf = find_leaf(key);
    auto existing = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    if (existing != leaf->keys.end() && *existing == key) {
        const auto idx = static_cast<std::size_t>(std::distance(leaf->keys.begin(), existing));
        leaf->offsets[idx] = offset;
        return;
    }

    const auto idx = static_cast<std::size_t>(std::distance(
        leaf->keys.begin(), std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key)));
    leaf->keys.insert(leaf->keys.begin() + static_cast<std::ptrdiff_t>(idx), key);
    leaf->offsets.insert(leaf->offsets.begin() + static_cast<std::ptrdiff_t>(idx), offset);

    if (leaf->keys.size() > max_keys_) split_leaf(leaf);
}

void BPlusTree::split_leaf(Node* leaf) {
    Node* right = new Node(true);
    const std::size_t split = (leaf->keys.size() + 1) / 2;

    right->keys.assign(leaf->keys.begin() + static_cast<std::ptrdiff_t>(split), leaf->keys.end());
    right->offsets.assign(leaf->offsets.begin() + static_cast<std::ptrdiff_t>(split), leaf->offsets.end());
    leaf->keys.resize(split);
    leaf->offsets.resize(split);

    right->next = leaf->next;
    leaf->next = right;

    insert_into_parent(leaf, right->keys.front(), right);
}

BPlusTree::Node* BPlusTree::find_parent(Node* current, Node* child) const {
    if (!current || current->is_leaf) return nullptr;
    for (Node* candidate : current->children) {
        if (candidate == child) return current;
        if (!candidate->is_leaf) {
            if (Node* found = find_parent(candidate, child)) return found;
        }
    }
    return nullptr;
}

void BPlusTree::insert_into_parent(Node* left, int separator, Node* right) {
    if (left == root_) {
        Node* new_root = new Node(false);
        new_root->keys.push_back(separator);
        new_root->children.push_back(left);
        new_root->children.push_back(right);
        root_ = new_root;
        return;
    }

    Node* parent = find_parent(root_, left);
    if (!parent) throw std::runtime_error("B+ tree parent lookup failed");

    auto child_it = std::find(parent->children.begin(), parent->children.end(), left);
    const auto child_idx = static_cast<std::size_t>(std::distance(parent->children.begin(), child_it));
    parent->keys.insert(parent->keys.begin() + static_cast<std::ptrdiff_t>(child_idx), separator);
    parent->children.insert(parent->children.begin() + static_cast<std::ptrdiff_t>(child_idx + 1), right);

    if (parent->keys.size() > max_keys_) split_internal(parent);
}

void BPlusTree::split_internal(Node* node) {
    const std::size_t mid = node->keys.size() / 2;
    const int promote = node->keys[mid];

    Node* right = new Node(false);
    right->keys.assign(node->keys.begin() + static_cast<std::ptrdiff_t>(mid + 1), node->keys.end());
    right->children.assign(node->children.begin() + static_cast<std::ptrdiff_t>(mid + 1), node->children.end());

    node->keys.resize(mid);
    node->children.resize(mid + 1);

    insert_into_parent(node, promote, right);
}

bool BPlusTree::erase(int key) {
    Node* leaf = find_leaf(key);
    if (!leaf) return false;
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    if (it == leaf->keys.end() || *it != key) return false;
    const auto idx = static_cast<std::size_t>(std::distance(leaf->keys.begin(), it));
    leaf->keys.erase(it);
    leaf->offsets.erase(leaf->offsets.begin() + static_cast<std::ptrdiff_t>(idx));
    return true;
}

} // namespace minidb
