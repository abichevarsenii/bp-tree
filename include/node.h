
#pragma once

#include "bptree.h"
#include "tree_iterator.h"

#include <functional>
#include <iostream>
#include <memory>
#include <utility>

template <class Key, class Value, class Less>
class Inner_node;

template <class Key, class Value, class Less>
class Leaf;

template <class Key, class Value, class Less>
class BPTree;

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

    virtual std::tuple<Node *, Node &, std::size_t, bool> insert(Node * root, const std::pair<Key, Value> & value) = 0;

    virtual Node * split(Node * root) = 0;

    virtual std::pair<Node *, Node *> merge(Node * root) = 0;

    virtual void print() = 0;

    virtual void new_parent(Inner_node<Key, Value, Less> * new_parent) = 0;

    virtual std::tuple<iterator, Node *, Node *> erase(Node * root, const_iterator it) = 0;

    virtual std::tuple<iterator, Node *, Node *> erase(Node * root, iterator it) = 0;

    size_t size = 0;
    Inner_node<Key, Value, Less> * parent = nullptr;
    Leaf<Key, Value, Less> * right = nullptr;
    static constexpr std::size_t max_size =
            (BPTree<Key, Value, Less>::block_size - sizeof(size) - sizeof(parent) - sizeof(right)) /
            sizeof(std::pair<Key, Node *>);
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

    std::tuple<Node *, Node &, std::size_t, bool> insert(Node * root, const std::pair<Key, Value> & value) override
    {
        auto it = std::lower_bound(data.begin(), data.begin() + size, value, [](const auto & ihs, const auto & rhs) {
            return ihs.first < rhs.first;
        });
        const std::size_t i = it - data.begin();
        if (i == size || it->first != value.first) {

            if (i == 0) {
                auto res = parent;
                while (res != nullptr) {
                    auto it2 = std::lower_bound(res->data.begin(), res->data.begin() + res->size, value, [](const auto & ihs, const auto & rhs) {
                        return ihs.first < rhs.first;
                    });

                    it2->first = value.first;

                    if (it2 - res->data.begin() == 0) {
                        res = res->parent;
                    }
                    else {
                        res = nullptr;
                    }
                }
            }

            std::move_backward(it, data.begin() + size, data.begin() + size + 1);
            size++;
            *it = value;
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
        const std::size_t size1 = size / 2;
        const std::size_t size2 = size - size1;
        Leaf * leaf = new Leaf();
        std::move(data.begin() + size1, data.end(), leaf->data.begin());
        leaf->size = size2;
        leaf->right = std::move(right);
        right = leaf;
        size = size1;
        const Key & key = right->data[0].first;
        if (parent != nullptr) {
            root = parent->push(root, std::move(key), std::move(leaf));
        }
        else {
            root = new Inner_node<Key, Value, Less>(this, std::move(key), std::move(leaf));
        }
        return root;
    }

    void replace_left(Key key, Node * node, Leaf * next, size_t level)
    {
        auto res = dynamic_cast<Inner_node<Key, Value, Less> *>(node);
        if (res == nullptr) {
            return;
        }
        const auto rend = std::make_reverse_iterator(res->data.begin());
        const auto find = std::lower_bound(std::make_reverse_iterator(res->data.begin() + res->size), rend, key, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs;
        });
        if (find->first != res->data[0].first && find->first == key) {
            if (level == 1) {
                Leaf * leaf = dynamic_cast<Leaf *>((find + 1)->second);
                if (leaf != nullptr) {
                    leaf->right = next;
                }
            }
            else {
                replace_left(key, (find)->second, next, level - 1);
            }
        }
        else {
            if (node->parent != nullptr) {
                replace_left(key, node->parent, next, level + 1);
            }
        }
    }

    std::tuple<iterator, Node *, Node *> erase_impl(Node * root, size_t pos)
    {
        auto res_key = data[pos].first;
        for (std::size_t i = pos; i < data.size() - 1; ++i) {
            data[i] = data[i + 1];
        }
        size--;

        if (this == root) {
            return {iterator(*this, pos), root, nullptr};
        }

        if (size == 0) {
            replace_left(res_key, parent, right, 1);
            auto res1 = iterator();
            if (right != nullptr) {
                res1 = iterator(*right, pos);
            }
            right = nullptr;
            auto res2 = parent->push_erase(root, data[pos].first, res_key, nullptr);
            std::tuple<iterator, Node *, Node *> res = {res1, res2, this};
            return res;
        }

        if (size >= max_size / 2) {
            return {iterator(*this, pos), root, nullptr};
        }
        else {
            std::pair<Node *, Node *> res = merge(root);
            return {iterator(*this, pos), res.first, res.second};
        }
    }

    std::tuple<iterator, Node *, Node *> erase(Node * root, const_iterator it) override
    {
        return erase_impl(root, it.position());
    }

    std::tuple<iterator, Node *, Node *> erase(Node * root, iterator it) override
    {
        return erase_impl(root, it.position());
    }

    Node * push_erase(Node * root, Key key1, Key key2, Node * new_child) override
    {
        return parent->push_erase(root, key1, key2, new_child);
    }

    std::pair<Node *, Node *> merge(Node * root) override
    {
        Node * deleted = nullptr;
        if (right != nullptr) {
            Leaf * node1 = dynamic_cast<Leaf *>(right);
            if (size + node1->size >= max_size) {
                std::size_t new_size = (size + node1->size) / 2;
                std::size_t rest_size = (size + node1->size) - new_size;

                auto key2 = node1->data[0].first;

                if (new_size > size) {
                    std::move(node1->data.begin(), node1->data.begin() + (new_size - size), data.begin() + size);
                    std::move(node1->data.begin() + (new_size - size), node1->data.end(), node1->data.begin());
                }
                else {
                    std::move(node1->data.begin(), node1->data.begin() + node1->size, node1->data.begin() + (new_size - node1->size));
                    std::move(data.begin(), data.begin() + (new_size - node1->size), node1->data.begin());
                }

                auto res = parent;
                while (res != nullptr) {
                    auto it2 = std::lower_bound(res->data.begin(), res->data.begin() + res->size, key2, [](const auto & ihs, const auto & rhs) {
                        return ihs.first < rhs;
                    });

                    it2->first = node1->data[0].first;

                    if (it2 - res->data.begin() == 0) {
                        res = res->parent;
                    }
                    else {
                        res = nullptr;
                    }
                }

                size = new_size;
                node1->size = rest_size;
            }
            else {
                auto i = size;
                if (size > node1->size) {
                    i = size - node1->size;
                }
                std::copy(node1->data.begin(), node1->data.begin() + node1->size, data.begin() + size);
                size += node1->size;
                right = node1->right;
                deleted = node1;

                if (parent != nullptr && parent == root &&
                    parent->size == 1) {
                    return {root, deleted};
                }

                root = parent->push_erase(root, data[i].first, data[i].first, nullptr);

                if (size < max_size / 2) {
                    std::pair<Node *, Node *> res = merge(root);
                    root = res.first;
                    deleted = res.second;
                }
                return {root, deleted};
            }
        }
        else {
            if (parent != nullptr && parent == root &&
                parent->size == 1) {
                return {root, deleted};
            }

            if (parent != nullptr) {
                auto res = parent;
                if (res->size > 1) {
                    const auto rend = std::make_reverse_iterator(res->data.begin());
                    const auto it2 = std::lower_bound(std::make_reverse_iterator(res->data.begin() + res->size), rend, data[0], [](const auto & lts, const auto & rhs) {
                        return lts.first > rhs.first;
                    });
                    root = parent->push_erase(root, data[0].first, data[0].first, nullptr);
                    std::pair<Node *, Node *> res2 = (it2 + 1)->second->merge(root);
                    root = res2.first;
                    deleted = res2.second;
                }
            }
        }
        return {root, deleted};
    }

    void print() override
    {
        std::cout << "leaf size: " << size << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            std::cout << data.at(i).first << " VALUE " << std::endl;
        }
    }

    void new_parent(Inner_node<Key, Value, Less> * new_parent) override
    {
        this->parent = new_parent;
    }

    std::size_t size;
    Inner_node<Key, Value, Less> * parent = nullptr;
    Leaf<Key, Value, Less> * right = nullptr;
    static constexpr std::size_t max_size =
            (BPTree<Key, Value, Less>::block_size - sizeof(size) - sizeof(parent) - sizeof(right)) /
            sizeof(std::pair<Key, Value>);
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
        data[1] = std::pair<Key, Node *>(key, std::move(node2));
        data[0] = std::pair<Key, Node *>(node->data[0].first, std::move(node));
        data[0].second->new_parent(this);
        data[1].second->new_parent(this);
    }

    Inner_node(Leaf<Key, Value, Less> * node, const Key & key, Leaf<Key, Value, Less> * node2)
        : size(2)
    {
        data[1] = std::pair<Key, Node *>(key, std::move(node2));
        data[0] = std::pair<Key, Node *>(node->data[0].first, std::move(node));
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
        const std::size_t new_size = size / 2;
        const std::size_t rest_size = size - new_size;
        auto * new_node = new Inner_node(rest_size, std::move(data[new_size].second));
        Key split_key = data[new_size].first;
        std::move(data.begin() + new_size, data.end(), new_node->data.begin());
        for (auto i = data.begin() + new_size; i != data.end(); i++) {
            i->second->new_parent(new_node);
        }
        size = new_size;

        if (parent != nullptr) {
            root = parent->push(root, split_key, std::move(new_node));
        }
        else {
            root = new Inner_node(this, split_key, std::move(new_node));
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
            *it2 = std::make_pair(key, std::move(new_child));
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
                std::move(it2, data.end(), it2 - 1);
            }
            else {
                return root;
            }
            if (size < max_size / 2) {
                std::pair<Node *, Node *> res2 = merge(root);
                root = res2.first;
            }
            if (this != root && parent != nullptr) {
                return parent->push_erase(root, key1, key2, new_child);
            }
        }
        else {
            const auto it2 = std::lower_bound(data.begin(), data.begin() + size, key1, [](const auto & ihs, const auto & rhs) {
                return ihs.first < rhs;
            });

            size--;
            it2->second = new_child;
            if (size != 0) {
                std::move(it2, data.end(), it2 - 1);
            }
            (it2 - 1)->first = key2;
            if (size < max_size / 2) {
                std::pair<Node *, Node *> res2 = merge(root);
                root = res2.first;
            }
            if (parent != nullptr) {
                return parent->push_erase(root, key1, key2, new_child);
            }
        }
        return root;
    }

    void print() override
    {
        std::cout << "inner ---------------- size: " << size << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            std::cout << "\t" << data.at(i).first << " " << data.at(i).second << std::endl;
        }
        std::cout << "begin -------" << std::endl;
        for (std::size_t i = 0; i < size; ++i) {
            data.at(i).second->print();
        }
        std::cout << " --------------- end" << std::endl;
    }

    std::tuple<iterator, Node *, Node *> erase(Node * root, const_iterator it) override
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

    std::tuple<iterator, Node *, Node *> erase(Node * root, iterator it) override
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

    Node * find_left_or_right(Key key, Node * node, size_t level, bool left)
    {
        auto res = dynamic_cast<Inner_node<Key, Value, Less> *>(node);
        if (res == nullptr) {
            return nullptr;
        }
        const auto rend = std::make_reverse_iterator(res->data.begin());
        const auto find = std::lower_bound(std::make_reverse_iterator(res->data.begin() + res->size), rend, key, [](const auto & lts, const auto & rhs) {
            return lts.first > rhs;
        });
        if (find->first != res->data[0].first && find->first == key) {
            if (level == 1) {
                Node * left_node;
                if (left) {
                    left_node = (find + 1)->second;
                }
                else {
                    left_node = (find - 1)->second;
                }
                return left_node;
            }
            else {
                find_left_or_right(key, (find)->second, level - 1, left);
            }
        }
        else {
            if (node->parent != nullptr) {
                find_left_or_right(key, node->parent, level + 1, left);
            }
        }
        return nullptr;
    }

    std::pair<Node *, Node *> merge(Node * root) override
    {
        Node * deleted = nullptr;
        Node * new_node = find_left_or_right(data[0].first, parent, 1, true);
        if (new_node == nullptr) {
            new_node = find_left_or_right(data[0].first, parent, 1, false);
        }

        if (new_node != nullptr) {
            auto * node1 = dynamic_cast<Inner_node *>(new_node);
            if (size + node1->size >= max_size) {
                std::size_t new_size = (size + node1->size) / 2;
                std::size_t rest_size = (size + node1->size) - new_size;

                auto key2 = node1->data[0].first;

                if (new_size > size) {
                    std::move(node1->data.begin(), node1->data.begin() + (new_size - size), data.begin() + size);
                    std::move(node1->data.begin() + (new_size - size), node1->data.end(), node1->data.begin());
                }
                else {
                    std::move(node1->data.begin(), node1->data.begin() + node1->size, node1->data.begin() + (new_size - node1->size));
                    std::move(data.begin(), data.begin() + (new_size - node1->size), node1->data.begin());
                }

                auto res = parent;
                while (res != nullptr) {
                    auto it2 = std::lower_bound(res->data.begin(), res->data.begin() + res->size, key2, [](const auto & ihs, const auto & rhs) {
                        return ihs.first < rhs;
                    });

                    it2->first = node1->data[0].first;

                    if (it2 - res->data.begin() == 0) {
                        res = res->parent;
                    }
                    else {
                        res = nullptr;
                    }
                }

                size = new_size;
                node1->size = rest_size;
            }
            else {
                auto i = size;
                if (size > node1->size) {
                    i = size - node1->size;
                }
                std::copy(node1->data.begin(), node1->data.begin() + node1->size, data.begin() + size);
                size += node1->size;
                deleted = node1;

                if (parent != nullptr && parent == root &&
                    parent->size == 1) {
                    return {root, deleted};
                }

                root = parent->push_erase(root, data[i].first, data[i].first, nullptr);

                if (size < max_size / 2) {
                    std::pair<Node *, Node *> res = merge(root);
                    root = res.first;
                    deleted = res.second;
                }
                return {root, deleted};
            }
        }
        else {
            if (parent != nullptr && parent == root &&
                parent->size == 1) {
                return {root, deleted};
            }

            if (parent != nullptr) {
                auto res = parent;
                if (res->size > 1) {
                    const auto rend = std::make_reverse_iterator(res->data.begin());
                    const auto it2 = std::lower_bound(std::make_reverse_iterator(res->data.begin() + res->size), rend, data[0], [](const auto & lts, const auto & rhs) {
                        return lts.first > rhs.first;
                    });
                    root = parent->push_erase(root, data[0].first, data[0].first, nullptr);
                    std::pair<Node *, Node *> res2 = (it2 + 1)->second->merge(root);
                    root = res2.first;
                    deleted = res2.second;
                }
            }
        }
        return {root, deleted};
    }

    void new_parent(Inner_node<Key, Value, Less> * new_parent) override
    {
        this->parent = new_parent;
    }

    std::size_t size;
    Inner_node<Key, Value, Less> * parent = nullptr;
    static constexpr std::size_t max_size =
            (BPTree<Key, Value, Less>::block_size - sizeof(size) - sizeof(parent)) / sizeof(std::pair<Key, Node *>);
    std::array<std::pair<Key, Node *>, max_size> data;
};