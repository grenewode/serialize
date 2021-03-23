# Simple proof of concept for C++20 serialization

This example is built on top of [nlohmann json](https://github.com/nlohmann/json), and provides a moderately lightweight solution for registering members of serializable objects.

This system has the following drawbacks, however:
1. It is not `constexpr`, and cannot be, because it uses `static` variables inside functions when creating the serialization tables.
2. It is not threadsafe, although it probably could be
3. It requires the user to wrap the name of any member that should be serialized in a macro:
```cpp
struct Foo {
  // Tell the system that hello should be serialized
  int SERIALIZE(hello);
  int SERIALIZE(world) = 60;
};
```
4. It does not handle pointers, although it could be extended to do so in some cases
