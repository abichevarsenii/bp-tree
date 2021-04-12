#pragma once

template <class Key, class Value, class Less = std::less<Key>>
class BPTree
{
    static constexpr std::size_t block_size = 4096;

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    using iterator = ...;
    using const_iterator = ...;

    BPTree(std::initializer_list<std::pair<const Key, Value>>);
    BPTree(const BPTree &);
    BPTree(BPTree &&);

    iterator begin();
    const_iterator cbegin() const;
    const_iterator begin() const;
    iterator end();
    const_iterator cend() const;
    const_iterator end() const;

    bool empty() const;
    size_type size() const;
    void clear();

    size_type count(const Key &) const;
    bool contains(const Key &) const;
    std::pair<iterator, iterator> equal_range(const Key &);
    std::pair<const_iterator, const_iterator> equal_range(const Key &) const;
    iterator lower_bound(const Key &);
    const_iterator lower_bound(const Key &) const;
    iterator upper_bound(const Key &);
    const_iterator upper_bound(const Key &) const;

    // 'at' method throws std::out_of_range if there is no such key
    Value & at(const Key &);
    const Value & at(const Key &) const;

    // '[]' operator inserts a new element if there is no such key
    Value & operator[](const Key &);
    Value & operator[](Key &&);

    std::pair<iterator, bool> insert(const Key &, const Value &); // NB: a digression from std::map
    std::pair<iterator, bool> insert(const Key &, Value &&);      // NB: a digression from std::map
    std::pair<iterator, bool> insert(Key &&, Value &&);           // NB: a digression from std::map
    void insert(std::initializer_list<value_type>);
    iterator erase(const_iterator);
    void erase(const_iterator, const_iterator);
    size_type erase(const Key &);
};
