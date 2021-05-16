#pragma once

#include "node.h"
#include "tree_iterator.h"

#include <functional>
#include <set>

template <class Key, class Value, class Less = std::less<Key>>
class BPTree
{
    static constexpr std::size_t block_size = 4096; //поставить обратно

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    using iterator = tree_iterator<Key, Value, Less, false>;      // убрать
    using const_iterator = tree_iterator<Key, Value, Less, true>; // убрать

    BPTree()
        : size_value(0)
        , root(new Leaf<Key, Value, Less>)
        , first_leaf(dynamic_cast<Leaf<Key, Value, Less> *>(root))
    {
        std::cout << "Max leaf size: " << Leaf<Key, Value, Less>::max_size << std::endl;
        std::cout << "Max inner size: " << Inner_node<Key, Value, Less>::max_size << std::endl;
    }

    BPTree(std::initializer_list<std::pair<const Key, Value>> list)
        : size_value(0)
        , root(new Leaf<Key, Value, Less>)
        , first_leaf(dynamic_cast<Leaf<Key, Value, Less> *>(root))
    {
        auto res_list = list;
        for (auto i = res_list.begin(); i != res_list.end(); ++i) {
            this->insert(i->first, i->second);
        }
        std::cout << "Max leaf size: " << Leaf<Key, Value, Less>::max_size << std::endl;
        std::cout << "Max inner size: " << Inner_node<Key, Value, Less>::max_size << std::endl;
    }

    BPTree(const BPTree & tree)
        : size_value(tree.size_value)
        , root(tree.root)
        , first_leaf(tree.first_leaf)
    {
        std::cout << "Max leaf size: " << Leaf<Key, Value, Less>::max_size << std::endl;
        std::cout << "Max inner size: " << Inner_node<Key, Value, Less>::max_size << std::endl;
    }

    BPTree(BPTree && tree)
        : size_value(tree.size_value)
        , root(tree.root)
        , first_leaf(tree.first_leaf)
    {
        std::cout << "Max leaf size: " << Leaf<Key, Value, Less>::max_size << std::endl;
        std::cout << "Max inner size: " << Inner_node<Key, Value, Less>::max_size << std::endl;
    }
    //BPTree();

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
        ~BPTree(); // переделать
        BPTree();
    }

    const_iterator find(const Key & key) const
    {
        const auto res = root->lower(key);
        //const Leaf<Key,Value,Less> * res2 = dynamic_cast<Leaf<Key,Value,Less>*>(res.first);
        /*for (int i = 0; i < res.first.size; ++i) {
            std::cout << ": " << res.first.data[i].first << std::endl;
        }*/
        const auto res2 = dynamic_cast<Leaf<Key, Value, Less> &>(res.first);
        //std::cout << dynamic_cast<Leaf<Key,Value,Less>*>(&res.first)->data[res.second].first << " " << res.second << std::endl;
        if (res2.data.at(res.second).first == key) {
            return const_iterator(res.first, res.second);
        }
        else {
            return cend();
        }
    }

    iterator find(const Key & key)
    {
        const auto res = root->lower(key);
        //const Leaf<Key,Value,Less> * res2 = dynamic_cast<Leaf<Key,Value,Less>*>(res.first);
        /*for (int i = 0; i < res.first.size; ++i) {
            std::cout << ": " << res.first.data[i].first << std::endl;
        }*/
        const auto res2 = dynamic_cast<Leaf<Key, Value, Less> &>(res.first);
        //std::cout << dynamic_cast<Leaf<Key,Value,Less>*>(&res.first)->data[res.second].first << " " << res.second << std::endl;
        if (res2.data.at(res.second).first == key) {
            return iterator(res.first, res.second);
        }
        else {
            return end();
        }
    }

    const_iterator find_const(const Key & key) const
    {
        const auto res = root->lower(key);
        //const Leaf<Key,Value,Less> * res2 = dynamic_cast<Leaf<Key,Value,Less>*>(res.first);
        /*for (int i = 0; i < res.first.size; ++i) {
            std::cout << ": " << res.first.data[i].first << std::endl;
        }*/
        const auto res2 = dynamic_cast<Leaf<Key, Value, Less> &>(res.first);
        //std::cout << dynamic_cast<Leaf<Key,Value,Less>*>(&res.first)->data[res.second].first << " " << res.second << std::endl;
        if (res2.data.at(res.second).first == key) {
            return const_iterator(res.first, res.second);
        }
        else {
            return cend();
        }
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
        //std::cout << "FFFFFF: " << find_const(key)->first << " " << std::endl;
        if (!(find_const(key) == end())) {
            return find_const(key)->first == key;
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
        return find_const(key)->second;
    }

    // '[]' operator inserts a new element if there is no such key
    Value & operator[](const Key & key)
    {
        auto res = find(key);
        if (res == end()) {
            //auto res2 = insert(key, 0).first;
            return (insert(key, Value()).first)->second;
        }
        return res->second;
    }

    Value & operator[](Key && key)
    {
        auto res = find(key);
        if (res == end()) {
            return (insert(key, Value()).first)->second;
        }
        return res->second;
    }

    using iter = typename std::initializer_list<std::pair<Key, Value>>::const_iterator;
    std::pair<iter, bool> insert(iter start, iter end)
    {
        for (auto i = start; i != end; i++) {
            insert((*i).first, (*i).second);
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

        const auto res = root->insert(root, std::pair<Key, Value>(key, std::move(value)));
        root = std::get<0>(res);
        if (std::get<3>(res)) {
            size_value++;
        }
        return std::pair<iterator, bool>(iterator(std::get<1>(res), std::get<2>(res)), std::get<3>(res));
    }

    std::pair<iterator, bool> insert(Key && key, Value && value)
    {
        const std::tuple<Node<Key, Value, Less> *, Node<Key, Value, Less> &, size_t, bool> res = root->insert(root,
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
        auto res = root->erase(root, it);
        root = res.second;
        size_value--;
        return res.first;
    }

    iterator erase(iterator it)
    {
        auto res = root->erase(root, it);
        root = res.second;
        size_value--;
        return res.first;
    }

    iterator erase(const_iterator begin, const_iterator end)
    {
        iterator res;
        //std::size_t pos = std::distance(begin, end);
        std::set<Key> set;
        for (auto j = begin; j != end; ++j) {
            set.insert(j->first);
        }

        for (auto j = set.begin(); j != set.end(); ++j) {
            res = erase(find_const(*j));
        }
        return res;
    }

    size_type erase(const Key & key)
    {
        const_iterator f = find_const(key);
        if (f != cend()) {
            auto res = root->erase(root, f);
            root = res.second;
            size_value--;
            return 1;
        }
        else {
            return 0;
        }
    }

    static constexpr size_type get_block_size()
    {
        return block_size;
    }

    ~BPTree()
    {
        if (first_leaf != root) {
            delete root;
        }
        delete_tree(first_leaf);
    }

    void delete_tree(Leaf<Key, Value, Less> * tree)
    {
        if (tree->right != nullptr) {
            delete_tree(tree->right);
        }
        delete tree;
    }

    void print()
    {
        std::cout << "Tree:";
        root->print();
    }

private:
    size_t size_value;
    Node<Key, Value, Less> * root;
    Leaf<Key, Value, Less> * first_leaf;
};
