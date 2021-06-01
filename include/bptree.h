#pragma once

#include "node.h"
#include "tree_iterator.h"

#include <functional>
#include <map>
#include <set>

template <class Key, class Value, class Less = std::less<Key>>
class BPTree
{

public:
    static constexpr std::size_t block_size = 4096;
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    using iterator = tree_iterator<Key, Value, Less, false>;
    using const_iterator = tree_iterator<Key, Value, Less, true>;

    BPTree()
        : size_value(0)
        , root(new Leaf<Key, Value, Less>)
        , first_leaf(dynamic_cast<Leaf<Key, Value, Less> *>(root))
    {
    }

    BPTree(std::initializer_list<std::pair<const Key, Value>> list)
        : BPTree()
    {
        auto res_list = list;
        for (auto i = res_list.begin(); i != res_list.end(); ++i) {
            this->insert(i->first, i->second);
        }
    }

    BPTree(const BPTree & tree)
        : BPTree()
    {
        for (auto i = tree.begin(); i != tree.end(); ++i) {
            this->insert(i->first, i->second);
        }
    }

    BPTree & operator=(const BPTree & tree)
    {
        this->clear();
        for (auto i = tree.begin(); i != tree.end(); ++i) {
            this->insert(i->first, i->second);
        }
        return *this;
    }

    BPTree & operator=(BPTree && tree)
    {
        this->clear();
        delete root;
        this->size_value = tree.size_value;
        this->root = tree.root;
        this->first_leaf = tree.first_leaf;
        this->deleted_set = std::move(tree.deleted_set);
        tree.deleted_set = std::set<Node<Key, Value> *>();
        tree.size_value = 0;
        tree.root = new Leaf<Key, Value, Less>;
        tree.first_leaf = dynamic_cast<Leaf<Key, Value, Less> *>(this->root);
        return *this;
    }

    BPTree(BPTree && tree)
    {
        this->size_value = tree.size_value;
        this->root = tree.root;
        this->first_leaf = tree.first_leaf;
        this->deleted_set = std::move(tree.deleted_set);
        tree.deleted_set = std::set<Node<Key, Value> *>();
        tree.size_value = 0;
        tree.root = new Leaf<Key, Value, Less>;
        tree.first_leaf = dynamic_cast<Leaf<Key, Value, Less> *>(this->root);
    }

    iterator begin()
    {
        return iterator(*first_leaf, 0);
    };

    const_iterator cbegin() const
    {
        return const_iterator(*first_leaf, 0);
    };

    const_iterator begin() const
    {
        return const_iterator(*first_leaf, 0);
    };

    iterator end()
    {
        return iterator();
    };

    const_iterator cend() const
    {
        return const_iterator();
    };

    const_iterator end() const
    {
        return const_iterator();
    };

    bool empty() const
    {
        return size() == 0;
    };

    size_type size() const
    {
        return size_value;
    };

    void clear()
    {
        this->deleted_set.insert(this->root);
        this->deleted_set.insert(this->first_leaf);
        delete_tree(this->first_leaf, this->deleted_set);
        this->size_value = 0;
        this->root = new Leaf<Key, Value, Less>();
        this->first_leaf = dynamic_cast<Leaf<Key, Value, Less> *>(this->root);
        for (auto item : this->deleted_set) {
            delete item;
        }
        this->deleted_set = std::set<Node<Key, Value> *>();
    }

    template <class T>
    T find_impl(const Key & key) const
    {
        const auto pair = root->lower(key);
        const auto leaf = dynamic_cast<Leaf<Key, Value, Less> &>(pair.first);
        if (leaf[pair.second].first == key) {
            return T(pair.first, pair.second);
        }
        else {
            return T();
        }
    }

    const_iterator find(const Key & key) const
    {
        return find_impl<const_iterator>(key);
    }

    iterator find(const Key & key)
    {
        return find_impl<iterator>(key);
    }

    const_iterator find_const(const Key & key) const
    {
        return find_impl<const_iterator>(key);
    }

    size_type count(const Key & key) const
    {
        if (contains(key)) {
            return 1;
        }
        else {
            return 0;
        }
    }

    bool contains(const Key & key) const
    {
        auto res = find_const(key);
        if (!(res == end())) {
            return res->first == key;
        }
        else {
            return false;
        }
    }

    std::pair<iterator, iterator> equal_range(const Key & key)
    {
        return {lower_bound(key), upper_bound(key)};
    }

    std::pair<const_iterator, const_iterator> equal_range(const Key & key) const
    {
        return {lower_bound(key), upper_bound(key)};
    }

    iterator lower_bound(const Key & key)
    {
        const auto res = root->lower(key);
        return iterator(res.first, res.second);
    };

    const_iterator lower_bound(const Key & key) const
    {
        const auto res = root->lower(key);
        return const_iterator(res.first, res.second);
    };

    iterator upper_bound(const Key & key)
    {
        const auto res = root->upper(key);
        return iterator(res.first, res.second);
    };

    const_iterator upper_bound(const Key & key) const
    {
        const auto res = root->upper(key);
        return const_iterator(res.first, res.second);
    };

    // 'at' method throws std::out_of_range if there is no such key
    Value & at(const Key & key)
    {
        auto res = find(key);
        if (res == end()) {
            throw std::out_of_range("");
        }
        else {
            return res->second;
        }
    }

    const Value & at(const Key & key) const
    {
        auto res = find_const(key);
        if (res == end()) {
            throw std::out_of_range("");
        }
        else {
            return res->second;
        }
    }

    // '[]' operator inserts a new element if there is no such key
    Value & operator[](const Key & key)
    {
        return (insert(key, Value()).first)->second;
    }

    Value & operator[](Key && key)
    {
        return (insert(key, Value()).first)->second;
    }

    template <class iter>
    std::pair<iter, bool> insert(iter start, iter end)
    {
        for (auto i = start; i != end; i++) {
            insert(i->first, i->second);
        }
        return {start, true};
    }

    std::pair<iterator, bool> insert(const Key & key, const Value & value)
    {
        const auto res = root->insert(root, std::pair<Key, Value>(key, value));
        root = std::get<0>(res);
        if (std::get<3>(res)) {
            size_value++;
        }
        return std::pair<iterator, bool>(iterator(std::get<1>(res), std::get<2>(res)), std::get<3>(res));
    }

    std::pair<iterator, bool> insert(const Key & key, Value && value)
    {
        const auto res = root->insert_move(root, std::pair<Key, Value>(key, std::move(value)));
        root = std::get<0>(res);
        if (std::get<3>(res)) {
            size_value++;
        }
        return std::pair<iterator, bool>(iterator(std::get<1>(res), std::get<2>(res)), std::get<3>(res));
    }

    std::pair<iterator, bool> insert(Key && key, Value && value)
    {
        const auto res = root->insert_move(root,
                                           std::pair<Key, Value>(
                                                   std::move(
                                                           key),
                                                   std::move(
                                                           value)));
        root = std::get<0>(res);
        if (std::get<3>(res)) {
            size_value++;
        }
        return std::pair<iterator, bool>(iterator(std::get<1>(res), std::get<2>(res)), std::get<3>(res));
    }

    void insert(std::initializer_list<value_type> list)
    {
        for (auto i = list.begin(); i != list.end(); i++) {
            insert(i->first, i->second);
        }
    }

    iterator erase(const_iterator it)
    {
        if (it != cend()) {
            auto res = root->erase(root, it);
            root = std::get<1>(res);
            size_value--;
            deleted_set.insert(std::get<2>(res));
            return std::get<0>(res);
        }
        else {
            return end();
        }
    }

    iterator erase(iterator it)
    {
        if (it != end()) {
            auto res = root->erase(root, it);
            root = std::get<1>(res);
            size_value--;
            deleted_set.insert(std::get<2>(res));
            return std::get<0>(res);
        }
        else {
            return end();
        }
    }

    iterator erase(const_iterator begin, const_iterator end)
    {
        iterator res;
        std::set<Key> key_set;
        for (auto j = begin; j != end; ++j) {
            key_set.insert(j->first);
        }
        for (auto j = key_set.begin(); j != key_set.end(); ++j) {
            res = erase(find_const(*j));
        }
        return res;
    }

    size_type erase(const Key & key)
    {
        const_iterator f = find_const(key);
        if (f != cend()) {
            auto res = root->erase(root, f);
            root = std::get<1>(res);
            size_value--;
            deleted_set.insert(std::get<2>(res));
            return 1;
        }
        else {
            return 0;
        }
    }

    ~BPTree()
    {
        deleted_set.insert(root);
        deleted_set.insert(first_leaf);
        delete_tree(first_leaf, deleted_set);
        for (auto item : deleted_set) {
            delete item;
        }
        this->deleted_set = std::set<Node<Key, Value> *>();
    }

    void delete_tree(Leaf<Key, Value, Less> * tree, std::set<Node<Key, Value> *> & set)
    {
        if (tree == nullptr) {
            return;
        }
        if (tree->right != nullptr) {
            auto res = tree->parent;
            while (res != nullptr) {
                set.insert(res);
                res = res->parent;
            }
            delete_tree(tree->right, set);
        }
        set.insert(tree);
    }

    void print()
    {
        std::cout << "Tree: " << std::endl;
        std::cout << "Root: " << root << std::endl;
        std::cout << "First Leaf: " << first_leaf << std::endl;
        root->print();
    }

private:
    size_t size_value;
    Node<Key, Value, Less> * root;
    Leaf<Key, Value, Less> * first_leaf;
    std::set<Node<Key, Value> *> deleted_set;
};
