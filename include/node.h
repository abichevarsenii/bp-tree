
#pragma once
#include "tree_iterator.h"

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

template <class Key, class Value, class Less>
class Inner_node;

template <class Key, class Value, class Less>
class Leaf;

template <class Key, class Value, class Less = std::less<Key>>
class Node
{
    using iterator = tree_iterator<Key, Value, Less, false>;
    using const_iterator = tree_iterator<Key, Value, Less, true>;

public:
    virtual ~Node() = default;
    virtual std::pair<Node &, std::size_t> lower(const Key & key) = 0;
    virtual std::pair<Node &, std::size_t> upper(const Key & key) = 0;
    virtual Node * push(Node * root, Key key, Node * new_child) = 0;
    virtual Node * push_erase(Node * root, Key key1, Key key2, Node * new_child) = 0;
    virtual std::pair<Node &, std::size_t> start() = 0;
    virtual std::pair<Node &, std::size_t> end() = 0;
    virtual std::tuple<Node *, Node &, std::size_t, bool> insert(Node * root, const std::pair<Key, Value> & value) = 0;
    virtual Node * split(Node * root) = 0;
    virtual Node * merge(Node * root) = 0;
    virtual void print() = 0;
    virtual void new_parent(Node * new_parent) = 0;
    virtual std::pair<iterator, Node *> erase(Node * root, const_iterator it) = 0;
    virtual std::pair<iterator, Node *> erase(Node * root, iterator it) = 0;
    size_t size = 0;
    Node * parent = nullptr;
    Leaf<Key, Value, Less> * right = nullptr;
    static constexpr std::size_t max_size = (4096) / sizeof(std::pair<Key, Node *>);
    //static constexpr std::size_t max_size = 102;
    std::array<std::pair<Key, Node *>, max_size> data;
};

template <class Key, class Value, class Less>
class Leaf : public Node<Key, Value>
{
    using Node = Node<Key, Value>;
    using iterator = tree_iterator<Key, Value, Less, false>;
    using const_iterator = tree_iterator<Key, Value, Less, true>;

public:
    Leaf()
        : size(0)
    {
    }

    ~Leaf() = default;

    Node * push(Node * root, Key key, Node * new_child) override
    {
        return parent->push(root, key, std::move(new_child));
    }

    std::pair<Node &, std::size_t> lower(const Key & key) override
    {
        const auto it = std::lower_bound(data.begin(), data.begin() + size, key, [](const auto & ihs, const auto & rhs) {
            return ihs.first < rhs;
        });
        const std::size_t i = it - data.begin();
        if (i < size) {
            return std::pair<Node &, std::size_t>(*this, i);
        }
        return std::pair<Node &, std::size_t>(*this, size);
    }

    std::pair<Node &, std::size_t> upper(const Key & key) override
    {
        const auto it = std::upper_bound(data.begin(), data.begin() + size, key, [](const auto & ihs, const auto & rhs) {
            return ihs < rhs.first;
        });
        const std::size_t i = it - data.begin();
        if (i < size) {
            return std::pair<Node &, std::size_t>(*this, i);
        }
        return std::pair<Node &, std::size_t>(*this, size);
    }

    auto & operator[](const std::size_t i) const
    {
        return data.at(i);
    }

    auto & operator[](const std::size_t i)
    {
        return data.at(i);
    }

    std::pair<Node &, std::size_t> start() override
    {
        return std::pair<Node &, std::size_t>(*this, 0); //переделать
    }

    std::pair<Node &, std::size_t> end() override
    {
        return std::pair<Node &, std::size_t>(*this, size - 1); //переделать
    }

    std::tuple<Node *, Node &, std::size_t, bool> insert(Node * root, const std::pair<Key, Value> & value) override
    {

        auto it = std::lower_bound(data.begin(), data.begin() + size, value, [](const auto & ihs, const auto & rhs) {
            return ihs.first < rhs.first;
        });
        const std::size_t i = it - data.begin();
        if (i == size || it->first != value.first) {

            if (i == 0) {
                //std::cout << "PUSH: " << value.first << std::endl;
                auto res = dynamic_cast<Inner_node<Key, Value, Less> *>(parent);
                while (res != nullptr) {
                    auto it2 = std::lower_bound(res->data.begin(), res->data.begin() + res->size, value, [](const auto & ihs, const auto & rhs) {
                        return ihs.first < rhs.first;
                    });

                    it2->first = value.first;

                    if (it2 - res->data.begin() == 0) {
                        res = dynamic_cast<Inner_node<Key, Value, Less> *>(res->parent);
                    }
                    else {
                        res = nullptr;
                    }
                }
            }

            std::move_backward(it, data.begin() + size, data.begin() + size + 1);

            *it = value;
            ++size;
            if (size < max_size) {
                return {root, *this, i, true};
            }
            else {
                root = split(root);
                if (i < size) {
                    return {root, *this, i, true};
                }
                else {
                    return {root, *right, i - size, true};
                }
            }
        }
        else {
            return {root, *this, i, false};
        }
    }

    Node * split(Node * root) override
    {

        const std::size_t new_size = size / 2;
        const std::size_t rest_size = size - new_size;
        Leaf * new_leaf = new Leaf();
        std::move(data.begin() + new_size, data.end(), new_leaf->data.begin());
        new_leaf->size = rest_size;
        new_leaf->right = right;
        right = new_leaf;
        size = new_size;
        const Key & split_key = right->data[0].first;

        //std::cout << split_key << " " << new_leaf->data[0].first << std::endl;

        if (parent != nullptr) {
            root = parent->push(root, split_key, new_leaf);
        }
        else {
            root = new Inner_node<Key, Value, Less>(this, split_key, new_leaf);
        }
        return root;
    }

    std::pair<iterator, Node *> erase(Node * root, const_iterator it) override
    {

        auto res_key = it.position();

        for (std::size_t i = it.position(); i < data.size() - 1; ++i) {
            data[i] = data[i + 1];
        }
        size--;

        if (size == 0) {
            return {iterator(*this, it.position()), push_erase(root, data[res_key].first, data[res_key].first, nullptr)};
            //return ;
        }

        if (this == root) {
            return {iterator(*this, it.position()), root};
        }
        //std::cout << size << std::endl;
        if (size >= max_size / 2) {

            return {iterator(*this, it.position()), root};
        }
        else {

            root = merge(root);
            return {iterator(*this, it.position()), root};
        }
    }

    std::pair<iterator, Node *> erase(Node * root, iterator it) override
    {
        auto res_key = it.position();
        for (std::size_t i = it.position(); i < data.size() - 1; ++i) {
            data[i] = data[i + 1];
        }
        size--;

        if (this == root) {
            return {iterator(*this, it.position()), root};
        }

        if (size == 0) {
            return {iterator(*this, it.position()), push_erase(root, data[res_key].first, data[res_key].first, nullptr)};
            //return ;
        }

        if (size >= max_size / 2) {
            return {iterator(*this, it.position()), root};
        }
        else {
            root = merge(root);

            return {iterator(*this, it.position()), root};
        }
    }

    Node * push_erase(Node * root, Key key1, Key key2, Node * new_child) override
    {
        return parent->push_erase(root, key1, key2, new_child);
    }

    Node * merge(Node * root) override
    {
        if (right != nullptr) {
            Leaf * node1 = dynamic_cast<Leaf *>(right);
            if (size + node1->size >= max_size) {
                std::size_t new_size = (size + node1->size) / 2;
                std::size_t rest_size = (size + node1->size) - new_size;

                //auto key1 = data[0].first;
                auto key2 = node1->data[0].first;

                //std::cout << key1 << " 9999999999999999999999999" << key2 << std::endl;

                if (new_size > size) {
                    std::move(node1->data.begin(), node1->data.begin() + (new_size - size), data.begin() + size);
                    std::move(node1->data.begin() + (new_size - size), node1->data.end(), node1->data.begin());
                }
                else {
                    std::move(node1->data.begin(), node1->data.begin() + node1->size, node1->data.begin() + (new_size - node1->size));
                    std::move(data.begin(), data.begin() + (new_size - node1->size), node1->data.begin());
                }

                auto res = dynamic_cast<Inner_node<Key, Value, Less> *>(parent);
                while (res != nullptr) {
                    auto it2 = std::lower_bound(res->data.begin(), res->data.begin() + res->size, key2, [](const auto & ihs, const auto & rhs) {
                        return ihs.first < rhs;
                    });

                    it2->first = node1->data[0].first;

                    if (it2 - res->data.begin() == 0) {
                        res = dynamic_cast<Inner_node<Key, Value, Less> *>(res->parent);
                    }
                    else {
                        res = nullptr;
                    }
                }

                size = new_size;
                node1->size = rest_size;
            }
            else {
                //std::cout << "jghjflhyvfyu"  << std::endl;
                std::move(node1->data.begin(), node1->data.begin() + node1->size, data.begin() + size);
                size += node1->size;
                right = std::move(node1->right);

                if (parent != nullptr && parent == root && dynamic_cast<Inner_node<Key, Value, Less> *>(parent)->size == 1) {
                    delete root;
                    return this;
                }

                auto i = size - node1->size;
                root = parent->push_erase(root, data[i].first, data[i].first, nullptr);

                if (size < max_size / 2) {
                    root = merge(root);
                }
                return root;
            }
        }
        else {
            if (parent != nullptr && parent == root && dynamic_cast<Inner_node<Key, Value, Less> *>(parent)->size == 1) {
                delete root;
                return this;
            }

            if (parent != nullptr) {
                auto res = dynamic_cast<Inner_node<Key, Value, Less> *>(parent);
                if (res->size > 1) {
                    const auto rend = std::make_reverse_iterator(res->data.begin());
                    const auto it2 = std::lower_bound(std::make_reverse_iterator(res->data.begin() + res->size), rend, data[0], [](const auto & lts, const auto & rhs) {
                        return lts.first > rhs.first;
                    });
                    root = (it2 + 1)->second->merge(root);
                }
            }
        }
        return root;
    }

    void print() override
    {
        std::cout << "leaf size: " << size << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            std::cout << data.at(i).first << " " << data.at(i).second << std::endl;
        }
    }

    void new_parent(Node * new_parent) override
    {
        this->parent = new_parent;
    }

    std::size_t size;
    Node * parent = nullptr;
    Leaf<Key, Value, Less> * right = nullptr;
    //static constexpr std::size_t max_size = (4096) / sizeof(std::pair<Key, Value>);
    static constexpr std::size_t max_size = 256;
    std::array<std::pair<Key, Value>, max_size> data;
};
template <class Key, class Value, class Less = std::less<Key>>
class Inner_node : public Node<Key, Value>
{
    //
    using Node = Node<Key, Value>;
    using iterator = tree_iterator<Key, Value, Less, false>;
    using const_iterator = tree_iterator<Key, Value, Less, true>;

public:
    Inner_node()
        : size(0)
    {
    }

    Inner_node(std::size_t size, Node * node)
        : size(size)
    {
        data[0] = std::pair<Key, Node *>(node->data[0].first, node);
        data[0].second->new_parent(this);
    }

    Inner_node(Inner_node * node, const Key & key, Inner_node * node2)
        : size(2)
    {
        data[1] = std::pair<Key, Node *>(key, node2);
        data[0] = std::pair<Key, Node *>(node->data[0].first, node);
        data[0].second->new_parent(this);
        data[1].second->new_parent(this);
    }

    Inner_node(Leaf<Key, Value, Less> * node, const Key & key, Leaf<Key, Value, Less> * node2)
        : size(2)
    {
        data[1] = std::pair<Key, Node *>(key, node2);
        data[0] = std::pair<Key, Node *>(node->data[0].first, node);
        data[0].second->new_parent(this);
        data[1].second->new_parent(this);
    }

    ~Inner_node() = default;

    std::pair<Node &, std::size_t> lower(const Key & key) override
    {
        const auto rend = std::make_reverse_iterator(data.begin());
        const auto it = std::lower_bound(std::make_reverse_iterator(data.begin() + size), rend, key, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs;
        });
        if (it != rend) {
            return it->second->lower(key);
        }
        return data.at(0).second->lower(key);
    }

    std::pair<Node &, std::size_t> upper(const Key & key) override
    {
        const auto rend = std::make_reverse_iterator(data.begin());
        const auto it = std::lower_bound(std::make_reverse_iterator(data.begin() + size), rend, key, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs;
        });
        if (it != rend) {
            return it->second->upper(key);
        }
        return data.at(0).second->upper(key);
    }

    std::pair<Node &, std::size_t> start() override
    {
        return std::pair<Node &, std::size_t>(*this, 0);
    }

    std::pair<Node &, std::size_t> end() override
    {
        return std::pair<Node &, std::size_t>(*this, size);
    }

    std::tuple<Node *, Node &, std::size_t, bool> insert(Node * root, const std::pair<Key, Value> & value) override
    {
        const auto rend = std::make_reverse_iterator(data.begin());
        const auto it = std::lower_bound(std::make_reverse_iterator(data.begin() + size), rend, value, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs.first;
        });
        if (it != rend) {
            return it->second->insert(root, value);
        }
        return data[0].second->insert(root, value);
    }

    Node * split(Node * root) override
    {
        //изменить

        const std::size_t new_size = size / 2;
        const std::size_t rest_size = size - new_size;
        auto * new_node = new Inner_node(rest_size, data[new_size].second);
        std::move(data.begin() + new_size, data.end(), new_node->data.begin());

        for (auto i = data.begin() + new_size; i != data.end(); i++) {
            i->second->new_parent(new_node);
        }
        size = new_size;
        Key split_key = std::move(data[size].first);
        if (parent != nullptr) {
            root = parent->push(root, std::move(split_key), new_node);
        }
        else {
            root = new Inner_node(this, split_key, new_node);
        }
        return root;
    }

    Node * push(Node * root, Key key, Node * new_child) override
    {
        const auto it2 = std::lower_bound(data.begin(), data.begin() + size, key, [](const auto & ihs, const auto & rhs) {
            return ihs.first < rhs;
        });

        const std::size_t i2 = it2 - data.begin();
        if (i2 == size || it2->first != key) {

            std::move_backward(it2, data.begin() + size, data.begin() + size + 1);
            *it2 = std::make_pair(std::move(key), new_child);
            it2->second->new_parent(this);
        }
        ++size;
        if (size < max_size) {
            return root;
        }
        else {
            return split(root);
        }
    }

    Node * push_erase(Node * root, Key key1, Key key2, Node * new_child) override
    {

        if (new_child == nullptr) {
            const auto it2 = std::lower_bound(data.begin(), data.begin() + size, key1, [](const auto & ihs, const auto & rhs) {
                return ihs.first < rhs;
            });

            size--;
            if (size != 0) {
                //delete it2->second;
                std::move(it2, data.end(), it2 - 1);
            }
            else {
                return root;
            }
            if (size < max_size / 2) {
                root = merge(root);
            }
            if (parent != nullptr) {
                return push_erase(root, key1, key2, new_child);
            }
        }
        else {
            const auto it2 = std::lower_bound(data.begin(), data.begin() + size, key1, [](const auto & ihs, const auto & rhs) {
                return ihs.first < rhs;
            });

            size--;
            it2->second = new_child;
            if (size != 0) {
                //delete it2->second;
                std::move(it2, data.end(), it2 - 1);
            }
            (it2 - 1)->first = key1;
            if (size < max_size / 2) {
                root = merge(root);
            }
            if (parent != nullptr) {
                return push_erase(root, key1, key2, new_child);
            }
        }
        return root;
    }

    void print() override
    {
        std::cout << "inner ----------------" << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            std::cout << "\t" << data.at(i).first << " " << data.at(i).second << std::endl;
        }
        std::cout << "begin -------" << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            data.at(i).second->print();
        }
        std::cout << " --------------- end" << std::endl;
    }

    std::pair<iterator, Node *> erase(Node * root, const_iterator it) override
    {
        const auto rend = std::make_reverse_iterator(data.begin());
        const auto res = std::lower_bound(std::make_reverse_iterator(data.begin() + size), rend, *it, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs.first;
        });
        if (res != rend) {
            return res->second->erase(root, it);
        }
        return data[size - 1].second->erase(root, it);
    }

    std::pair<iterator, Node *> erase(Node * root, iterator it) override
    {
        const auto rend = std::make_reverse_iterator(data.begin());
        const auto res = std::lower_bound(std::make_reverse_iterator(data.begin() + size), rend, *it, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs.first;
        });
        if (res != rend) {
            return res->second->erase(root, it);
        }
        return data[size - 1].second->erase(root, it);
    }

    Node * merge(Node * root) override
    {
        return root;
    }

    void new_parent(Node * new_parent) override
    {
        this->parent = new_parent;
    }

    std::size_t size;
    Node * parent = nullptr;
    Node * right = nullptr;
    static constexpr std::size_t max_size = (4096) / sizeof(std::pair<Key, Node *>);
    //static constexpr std::size_t max_size = 102;
    std::array<std::pair<Key, Node *>, max_size> data;
};