#include <iostream>
#include "serialize.hpp"

struct Foo {
  int SERIALIZE(hello);
  int SERIALIZE(world) = 60;
};

int main() {
  Foo foo;
  std::cout << serialize(foo).dump(1) << std::endl;

  auto bar = deserialize<Foo>(R"({
    "hello": 15,
    "world": -5
  })"_json);

  std::cout << "hello: " << bar.hello << std::endl
            << "world: " << bar.world << std::endl;
}