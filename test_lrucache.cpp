//#include "utils/LRUCache.h"
//#include <iostream>
//#include <string>
//#include <cassert>
//using std::string;
//using std::cout;
//
//static int passed = 0;
//static int failed = 0;
//#define CHECK(cond, name) \
//    do { \
//        if (cond) { ++passed; cout << "  [PASS] " << (name) << "\n"; } \
//        else { ++failed; cout << "  [FAIL] " << (name) << "\n"; } \
//    } while (0)
//
//static void test_basic_put_get() {
//    cout << "test_basic_put_get:\n";
//    LRUCache<string, int> cache(3);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.put("c", 3);
//    int* a = cache.get("a");
//    int* b = cache.get("b");
//    int* c = cache.get("c");
//    CHECK(a != nullptr && *a == 1, "get a returns 1");
//    CHECK(b != nullptr && *b == 2, "get b returns 2");
//    CHECK(c != nullptr && *c == 3, "get c returns 3");
//    CHECK(cache.size() == 3, "size is 3");
//}
//
//static void test_miss_returns_null() {
//    cout << "test_miss_returns_null:\n";
//    LRUCache<string, int> cache(2);
//    cache.put("x", 100);
//    CHECK(cache.get("y") == nullptr, "missing key returns nullptr");
//    CHECK(cache.contains("x") == true, "contains true for present key");
//    CHECK(cache.contains("y") == false, "contains false for missing key");
//}
//
//static void test_eviction_lru_order() {
//    cout << "test_eviction_lru_order:\n";
//    LRUCache<string, int> cache(2);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.put("c", 3);
//    CHECK(cache.get("a") == nullptr, "a evicted after capacity exceeded");
//    CHECK(cache.get("b") != nullptr, "b still present");
//    CHECK(cache.get("c") != nullptr, "c still present");
//    CHECK(cache.size() == 2, "size stays at capacity");
//}
//
//static void test_get_promotes_to_front() {
//    cout << "test_get_promotes_to_front:\n";
//    LRUCache<string, int> cache(3);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.put("c", 3);
//    (void)cache.get("a");
//    cache.put("d", 4);
//    CHECK(cache.get("a") != nullptr, "a kept (recently accessed)");
//    CHECK(cache.get("b") == nullptr, "b evicted (was LRU)");
//    CHECK(cache.get("c") != nullptr, "c kept");
//    CHECK(cache.get("d") != nullptr, "d inserted");
//}
//
//static void test_put_overwrite_does_not_grow() {
//    cout << "test_put_overwrite_does_not_grow:\n";
//    LRUCache<string, int> cache(2);
//    cache.put("k", 1);
//    cache.put("k", 2);
//    CHECK(cache.size() == 1, "overwrite keeps size at 1");
//    int* v = cache.get("k");
//    CHECK(v != nullptr && *v == 2, "overwrite stores new value");
//}
//
//static void test_put_overwrite_promotes() {
//    cout << "test_put_overwrite_promotes:\n";
//    LRUCache<string, int> cache(2);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.put("a", 99);
//    cache.put("c", 3);
//    CHECK(cache.get("a") != nullptr, "a kept after overwrite-promote");
//    CHECK(cache.get("b") == nullptr, "b evicted");
//    int* a = cache.get("a");
//    CHECK(a != nullptr && *a == 99, "a holds overwritten value");
//}
//
//static void test_remove() {
//    cout << "test_remove:\n";
//    LRUCache<string, int> cache(3);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.put("c", 3);
//    CHECK(cache.remove("b") == true, "remove returns true on hit");
//    CHECK(cache.remove("z") == false, "remove returns false on miss");
//    CHECK(cache.contains("b") == false, "b is gone after remove");
//    CHECK(cache.size() == 2, "size decreases after remove");
//    cache.put("d", 4);
//    cache.put("e", 5);
//    CHECK(cache.size() == 3, "size back to capacity");
//    CHECK(cache.contains("a") == false, "a evicted");
//    CHECK(cache.contains("c") == true, "c kept");
//    CHECK(cache.contains("d") == true, "d present");
//    CHECK(cache.contains("e") == true, "e present");
//}
//
//static void test_clear() {
//    cout << "test_clear:\n";
//    LRUCache<string, int> cache(3);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    cache.clear();
//    CHECK(cache.size() == 0, "size 0 after clear");
//    CHECK(cache.empty(), "empty true after clear");
//    CHECK(cache.get("a") == nullptr, "a gone after clear");
//    cache.put("x", 42);
//    CHECK(cache.size() == 1, "usable after clear");
//    int* x = cache.get("x");
//    CHECK(x != nullptr && *x == 42, "post-clear insert works");
//}
//
//static void test_capacity_one() {
//    cout << "test_capacity_one:\n";
//    LRUCache<string, int> cache(1);
//    cache.put("a", 1);
//    cache.put("b", 2);
//    CHECK(cache.get("a") == nullptr, "a evicted in capacity-1 cache");
//    CHECK(cache.get("b") != nullptr, "b present");
//    CHECK(cache.size() == 1, "size is 1");
//}
//
//static void test_capacity_zero_clamped_to_one() {
//    cout << "test_capacity_zero_clamped_to_one:\n";
//    LRUCache<string, int> cache(0);
//    CHECK(cache.capacity() == 1, "capacity 0 clamped to 1");
//    cache.put("x", 1);
//    CHECK(cache.size() == 1, "still functional with clamped capacity");
//}
//
//static void test_pointer_value_no_delete_on_evict() {
//    cout << "test_pointer_value_no_delete_on_evict:\n";
//    LRUCache<int, int*> cache(1);
//    int* alive = new int(123);
//    cache.put(1, alive);
//    cache.put(2, new int(456));
//    *alive = 999;
//    CHECK(*alive == 999, "evicted pointer still valid (cache did not delete)");
//    delete alive;
//    int** slot = cache.get(2);
//    if (slot != nullptr) delete* slot;
//}
//
//int main() {
//    cout << "Running LRUCache tests...\n\n";
//    test_basic_put_get();
//    test_miss_returns_null();
//    test_eviction_lru_order();
//    test_get_promotes_to_front();
//    test_put_overwrite_does_not_grow();
//    test_put_overwrite_promotes();
//    test_remove();
//    test_clear();
//    test_capacity_one();
//    test_capacity_zero_clamped_to_one();
//    test_pointer_value_no_delete_on_evict();
//    cout << "\n==============================\n";
//    cout << "Passed: " << passed << "\n";
//    cout << "Failed: " << failed << "\n";
//    cout << "==============================\n";
//    return failed == 0 ? 0 : 1;
//}