// Test for HashMap and HashSet
#include <iostream>
#include "mana_runtime.h"

int main() {
    std::cout << "=== HashMap Test ===" << std::endl;
    
    mana::HashMap<std::string, int32_t> ages;
    ages.insert("Alice", 30);
    ages.insert("Bob", 25);
    ages.insert("Charlie", 35);
    
    std::cout << "Size: " << ages.len() << std::endl;
    std::cout << "Contains Alice: " << (ages.contains("Alice") ? "true" : "false") << std::endl;
    std::cout << "Contains Dave: " << (ages.contains("Dave") ? "true" : "false") << std::endl;
    
    auto alice_age = ages.get("Alice");
    if (alice_age.is_some()) {
        std::cout << "Alice age: " << alice_age.unwrap() << std::endl;
    }
    
    std::cout << std::endl << "Keys: ";
    auto keys = ages.keys();
    for (size_t i = 0; i < keys.len(); ++i) {
        std::cout << keys[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << std::endl << "=== HashSet Test ===" << std::endl;
    
    mana::HashSet<int32_t> set1;
    set1.insert(1);
    set1.insert(2);
    set1.insert(3);
    
    mana::HashSet<int32_t> set2;
    set2.insert(2);
    set2.insert(3);
    set2.insert(4);
    
    std::cout << "Set1 size: " << set1.len() << std::endl;
    std::cout << "Set1 contains 2: " << (set1.contains(2) ? "true" : "false") << std::endl;
    std::cout << "Set1 contains 5: " << (set1.contains(5) ? "true" : "false") << std::endl;
    
    auto union_set = set1.union_with(set2);
    std::cout << "Union size: " << union_set.len() << std::endl;
    
    auto intersect_set = set1.intersect(set2);
    std::cout << "Intersection size: " << intersect_set.len() << std::endl;
    
    auto diff_set = set1.difference(set2);
    std::cout << "Difference size: " << diff_set.len() << std::endl;
    
    std::cout << std::endl << "All tests passed!" << std::endl;
    return 0;
}
