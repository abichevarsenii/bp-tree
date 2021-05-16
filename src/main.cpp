#include "bptree.h"

#include <iostream>
#include <list>
#include <string>

int main()
{
    BPTree<int, std::string> tree;
    tree.insert(11, "11");
    tree.insert(12, "12");
    tree.insert(13, "13");
    tree.print();
    auto it = tree.erase(tree.find(12));
    auto it2 = tree.find(13);
    std::cout << it->first << " " << it->second << std::endl;
    std::cout << it2->first << " " << it2->second << std::endl;
    tree.print();
    tree.erase(tree.find_const(11));
    tree.print();
    tree.erase(tree.find_const(13));
    tree.print();
}
