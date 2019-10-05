#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <typeindex>
#include <unordered_set>
#include <vector>
#include "nlohmann/json.hpp"

namespace grenewode_serialize {

template <typename U>
struct base_property {
  const char* name;

  constexpr base_property(const char* name) : name{name} {}
  virtual ~base_property() = default;

  constexpr base_property(const base_property&) = default;
  constexpr base_property(base_property&&) = default;

  constexpr base_property& operator=(const base_property&) = default;
  constexpr base_property& operator=(base_property&&) = default;

  virtual nlohmann::json serialize(const U& object) = 0;

  virtual void deserialize(U& object, const nlohmann::json& in) = 0;
};

template <typename T>
using property_table_type = std::vector<std::unique_ptr<base_property<T>>>;

namespace INTERNAL {

using serializable_registry_type = std::unordered_set<std::type_index>;

serializable_registry_type& serializable_registry();

template <typename T>
void register_as_serializable() {
  static auto _just_once = ([] {
    serializable_registry().insert(typeid(T));
    return '\0';
  })();
}
template <typename T>
bool is_registered_as_serializable() {
  return serializable_registry().count(typeid(T)) > 0;
}

template <typename T>
property_table_type<T>& get_property_table() {
  static property_table_type<T> property_table = ([] {
    register_as_serializable<T>();
    return property_table_type<T>{};
  })();

  return property_table;
}
}  // namespace INTERNAL

template <typename T>
struct is_nlohmann_serializable {
 private:
  template <typename U>
  static constexpr auto test(void*)
      -> decltype(nlohmann::json{std::declval<U>()}, std::true_type{});

  template <typename U>
  static constexpr std::false_type test(...);

 public:
  static constexpr decltype(test<T>(nullptr)) value{};
};

template <typename T>
struct is_nlohmann_deserializable {
 private:
  template <typename U>
  static constexpr auto test(void*)
      -> decltype(std::declval<nlohmann::json>().template get<U>(),
                  std::true_type{});

  template <typename U>
  static constexpr std::false_type test(...);

 public:
  static constexpr decltype(test<T>(nullptr)) value{};
};

template <typename T>
nlohmann::json serialize(const T& object) {
  if constexpr (is_nlohmann_serializable<T>::value) {
    return object;
  } else {
    assert(INTERNAL::is_registered_as_serializable<T>());

    nlohmann::json json;

    auto&& property_table = INTERNAL::get_property_table<T>();

    for (auto&& entry : property_table) {
      auto v = entry->serialize(object);
      json[entry->name] = entry->serialize(object);
    }

    return json;
  }
}

template <typename T>
T deserialize(const nlohmann::json& j) {
  if constexpr (is_nlohmann_deserializable<T>::value) {
    return j.template get<T>();
  } else {
    assert(INTERNAL::is_registered_as_serializable<T>());

    T object{};

    auto&& property_table = INTERNAL::get_property_table<T>();

    for (auto&& entry : property_table) {
      entry->deserialize(object, j[entry->name]);
    }

    return object;
  }
}

template <auto>
struct property;

template <typename U, typename T, T U::*MEMBER>
struct property<MEMBER> : base_property<U> {
  static constexpr auto member{MEMBER};

  constexpr property(const char* name) : base_property<U>{name} {}

  constexpr property(const property&) = default;
  constexpr property(property&&) = default;

  constexpr property& operator=(const property&) = default;
  constexpr property& operator=(property&&) = default;

  virtual nlohmann::json serialize(const U& object) {
    return grenewode_serialize::serialize(object.*member);
  }

  virtual void deserialize(U& object, const nlohmann::json& in) {
    object.*member = grenewode_serialize::deserialize<T>(in);
  }
};

namespace INTERNAL {

struct PropertyInitializer {
  template <typename T>
  constexpr operator T() const {
    return T{};
  }

  template <typename T>
  T&& operator=(T&& value) const {
    return std::forward<T>(value);
  }
};

template <auto>
struct destructure_member_pointer;

template <typename U, typename T, T U::*MEMBER>
struct destructure_member_pointer<MEMBER> {
  using class_type = U;
  using member_type = T;
};

template <auto MEMBER>
PropertyInitializer make_property(const char* name) {
  using class_type = typename destructure_member_pointer<MEMBER>::class_type;

  static auto _just_once = ([name] {
    get_property_table<class_type>().push_back(
        std::make_unique<property<MEMBER>>(name));

    return '\0';
  })();

  return {};
}
}  // namespace INTERNAL

#define SERIALIZE(name)                                  \
  name = ::grenewode_serialize::INTERNAL::make_property< \
      &std::decay_t<decltype(*this)>::name>(#name)
}  // namespace grenewode_serialize

#endif
