#pragma once

#include "node.h"

#include <functional>
#include <type_traits>
#include <utility>

template <class Key, class Value, class Less>
class Inner_node;

template <class Key, class Value, class Less>
class Leaf;

template <class Key, class Value, class Less>
class Node;

template <class Key, class Value, class Less, bool constant>
class tree_iterator : public std::iterator<std::forward_iterator_tag, std::pair<Key, Value>>
{
public:
    using type = typename std::conditional_t<constant, const std::pair<Key, Value>, std::pair<Key, Value>>;
    using type_curr = typename std::conditional_t<constant, const Leaf<Key, Value, Less> *, Leaf<Key, Value, Less> *>;

    tree_iterator()
        : curr(nullptr)
        , pos(0){};

    tree_iterator(Node<Key, Value, Less> & curr, const std::size_t pos = 0)
        : curr(dynamic_cast<Leaf<Key, Value, Less> *>(&curr))
        , pos(pos)
    {
        end(curr, pos);
    }
    void end(Node<Key, Value, Less> & node, const size_t n)
    {
        auto * res = dynamic_cast<Leaf<Key, Value, Less> *>(&node);
        if (n == res->size) {
            tree_iterator::curr = dynamic_cast<Leaf<Key, Value, Less> *>(res->right);
            tree_iterator::pos = 0;
        }
    };

    /*tree_iterator(const tree_iterator & iter)
        : curr(iter.curr)
        , pos(iter.pos)
    {
    }

    tree_iterator(tree_iterator<Key,Value,Less, false> & iter)
            : curr(iter.curr)
            , pos(iter.pos)
    {
    }*/

    tree_iterator & operator++()
    {
        pos++;
        if (pos == curr->size) {
            this->curr = dynamic_cast<Leaf<Key, Value, Less> *>(curr->right);
            this->pos = 0;
        }
        return *this;
    }

    tree_iterator operator++(int)
    {
        auto res = *this;
        operator++();
        return res;
    }

    type & operator*() const
    {
        return (*curr)[pos];
    }

    std::size_t position() const
    {
        return pos;
    }

    auto node()
    {
        return curr;
    }

    type & operator*()
    {
        return (*curr)[pos];
    }

    type * operator->()
    {
        return &(*curr)[pos];
    }

    friend bool operator==(const tree_iterator a, const tree_iterator b)
    {
        return a.curr == b.curr && a.pos == b.pos;
    }

    friend bool operator!=(const tree_iterator a, const tree_iterator b)
    {
        return a.curr != b.curr || a.pos != b.pos;
    }

private:
    type_curr curr;
    size_t pos = 0;
};
