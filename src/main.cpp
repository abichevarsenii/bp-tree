#include "bptree.h"

#include <iostream>
#include <list>
#include <string>

int main()
{
    BPTree<int, std::string> tree;
    /*

    std::size_t n = 111;
    for (std::size_t i = 19; i < n; ++i) {
        tree.insert(i, "a" + std::to_string(i));
    }
*/

    //tree.print();

    //for (std::size_t i = 51; i < 91; ++i) {
    //if (!tree.contains(std::to_string(i))){
    //std::cout << "НЕ ХВАТАЕТ: " << i << std::endl;
    //}
    // tree.erase(i);
    //tree.erase(tree.find_const(51),tree.find_const(91));
    //tree.print();
    // }
    //auto it = tree.erase(tree.find_const(51),tree.find_const(51));
    //tree.print();
    // tree.erase(51);
    //tree.print();
    //auto it2 = tree.find(91);
    // std::cout << it->first << " 999=> " << it->second<< "\n";
    /*  std::cout << it2->first << " 999=> " << it2->second<< "\n";

     for (const auto & [key, value] : tree) {
          std::cout << key << " => " << value << "\n";
      }
  */

    std::cout << sizeof(Node<int, std::string> *) << "\n";
    tree.insert(11, "11");
    tree.insert(12, "12");
    tree.insert(13, "13");
    tree.print();
    tree.erase(tree.find(12));
    for (const auto & [key, value] : tree) {
        std::cout << key << " => " << value << "\n";
    }
    tree.print();
    tree.erase(tree.find(11));
    for (const auto & [key, value] : tree) {
        std::cout << key << " => " << value << "\n";
    }
    tree.print();
    tree.erase(tree.find(13));
    for (const auto & [key, value] : tree) {
        std::cout << key << " => " << value << "\n";
    }
    tree.print();
    for (const auto & [key, value] : tree) {
        std::cout << key << " => " << value << "\n";
    }
    std::cout << "end" << std::endl;
}
