#include <any>
#include <iostream>
#include <map>

using namespace std;

int main(void) {

  map<string, any> bag;

  bag["a"] = 12;
  bag["b"] = 3.1f;
  bag["c"] = true;
  bag["d"] = 123.321;
  bag["e"] = 3u;
  bag["f"] = 113344LL;

  for (auto &elem : bag) {
    const char *type_name = elem.second.type().name();
    size_t type_id = elem.second.type().hash_code();
    // const char *type_name = typeid(elem.second).name();
    printf("\"%s\"\ttype_id: %llu\ttype: %s\n", elem.first.c_str(), type_id,
           type_name);
  }

  return 0;
}
