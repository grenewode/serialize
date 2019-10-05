#include <iostream>
#include <vector>
#include "serialize.hpp"

struct Foo {
  int SERIALIZE(hello);
  int SERIALIZE(world) = 60;

  std::string SERIALIZE(my_other_field);
  std::vector<std::string> SERIALIZE(my_other_field_vector) = std::vector{
      std::string{"Hello"}, std::string{"World"}};
};

struct Bar {
  int hello;
  int world;
};

int main() {
  Foo foo;
  foo.my_other_field = "THis is still a test";
  std::cout << grenewode_serialize::serialize(foo).dump(1) << std::endl;

  auto de_foo = grenewode_serialize::deserialize<Foo>(R"({
    "hello": 15,
    "world": -5,
    "my_other_field": "Hello World THIS IS A TEST!",
    "my_other_field_vector": ["Goodbye", "World"]
  })"_json);

  std::cout << "hello: " << de_foo.hello << std::endl
            << "world: " << de_foo.world << std::endl
            << "my_other_field: " << de_foo.my_other_field << std::endl
            << "my_other_field_vector: " << de_foo.my_other_field << std::endl;

  // Bar does not contain any serializable fields, so it will throw an exception
  // if you try to serialize it

  std::cout << grenewode_serialize::serialize(Bar{}).dump(1) << std::endl;
}