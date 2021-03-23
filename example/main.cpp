#include <iostream>
#include <vector>
#include "serialize.hpp"

struct Foo {
 private:
  int SERIALIZE(hello);

 public:
  int get_hello() const { return hello; }

  int SERIALIZE(world) = 60;

  std::string SERIALIZE(my_other_field);
  std::vector<std::string> SERIALIZE(my_other_field_vector) = std::vector{
      std::string{"Hello"}, std::string{"World"}};

  struct Inner {
    std::map<std::string, std::string> SERIALIZE(mapping);
  } SERIALIZE(inner);
};

struct Bar {
  int hello;
  int world;
};

int main() {
  Foo foo;
  foo.my_other_field = "THis is still a test";
  foo.inner.mapping["Hello"] = "World";

  std::cout << grenewode_serialize::serialize(foo).dump(1) << std::endl;

  auto de_foo = grenewode_serialize::deserialize<Foo>(R"({
    "hello": 15,
    "world": -5,
    "my_other_field": "Hello World THIS IS A TEST!",
    "my_other_field_vector": ["Goodbye", "World"],
    "inner": { "mapping": { "Hello": "World" } }
  })"_json);

  std::cout << "hello: " << de_foo.get_hello() << std::endl
            << "world: " << de_foo.world << std::endl
            << "my_other_field: " << de_foo.my_other_field << std::endl
            << "my_other_field_vector: " << de_foo.my_other_field << std::endl;

  // Bar does not contain any serializable fields, so it will throw an exception
  // if you try to serialize it

  // std::cout << grenewode_serialize::serialize(Bar{}).dump(1) << std::endl;
}
