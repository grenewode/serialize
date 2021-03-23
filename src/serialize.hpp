#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <algorithm>
#include <typeindex>
#include <vector>
#include "nlohmann/json.hpp"

namespace grenewode_serialize {

// This will be used later by the property class to do type-errased
// serialization
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

using serializable_registry_type = std::vector<std::type_index>;

serializable_registry_type& serializable_registry();

template <typename T>
void register_as_serializable() {
  // See below for an explanation of what's happening here. Basically, we want
  // to make sure that each type can can be serialized is only registered once
  static auto _just_once = ([] {
    serializable_registry().push_back(typeid(T));
    return '\0';
  })();
}

template <typename T>
bool is_registered_as_serializable() {
  auto&& registry = serializable_registry();
  return std::find(registry.begin(), registry.end(), typeid(T)) !=
         registry.end();
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
T& deserialize_to(const nlohmann::json& j, T& v) {
  // If the value can be deserialized with nlohmann json,
  // then go ahead and do that.
  //
  // Otherwise, we will use our own deserialization code
  if constexpr (is_nlohmann_deserializable<T>::value) {
    return j.get_to(v);
  } else {
    // We can't do this at compile time yet :(
    // Also, in a perfect world, we would be throwing
    // an exception here, but I'm lazy
    assert(INTERNAL::is_registered_as_serializable<T>());

    auto&& property_table = INTERNAL::get_property_table<T>();
    for (auto&& entry : property_table) {
      entry->deserialize(v, j[entry->name]);
    }

    return v;
  }
}

template <typename T>
T deserialize(const nlohmann::json& j) {
  // You might be wondering, why repeat ourselves here?
  // We do this because nlohmann::json can deserialize
  // types even if they do not have a default constructor
  // in some cases. We want to make sure that we take
  // advantage of this as much as possible
  if constexpr (is_nlohmann_deserializable<T>::value) {
    return j.template get<T>();
  } else {
    T object{};

    deserialize_to(j, object);

    return object;
  }
}

// This is the star of the show!
// Here we use dynamic dispatch to do the
// serialization and deserialization
//
// this is a very similar method to the one used by
// std::function
//
// Note the user of an auto template
//
// This is a very nice feature from c++17,
// which allows us to skip passing the class &
// member type as well as the member pointer value
template <auto>
struct property;

// Now we can specialize and capture the information
// about the class type (U) and the member type (T)
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

// This class exists only to be returned from the make_property function below.
// It does exactly nothing except for passing values through its
// assignment operation without changing them at all.
// Basically, we need it so that we can stick side-effects (so, statements)
// into a place where you could normally only have expressions (so, no
// side-effects)
struct PropertyInitializer {
  template <typename T>
  constexpr operator T() const {
    return T{};
  }

  template <typename T>
  constexpr T&& operator=(T&& value) const {
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
  // You might be wondering what is happening here.
  //
  // We need to register each member that will be deserialized, and we need to
  // make sure that this only happens at most ONCE per serializable member -
  // for the entire program!
  //
  // Since we are hacking the behaviour of initializers to invoke side-effects
  // during member initialization, if we just run the
  // get_property_table().push_back(property) then we will end up registering
  // the each member EVERY time the object is CONSTRUCTED. We could use
  // a set or a table here, but that would be very slow. It would be better
  // if the compiler could be smart enough to figure it out for us.
  //
  // C++ doesn't provide a GREAT way to do this, but it does provide an
  // OK way to do this - by using static function variables.
  //
  // _just_once is static - which means in will be initialized once & ONLY once
  // the first time this function is executed. By assiging it the value produced
  // by creating and calling a lambda, we can execute our side effects. Then we
  // just return a dummy value and get on with life
  //
  // You might be wondering, "wont the compiler just optimize the whole variable
  // & its initializer away since its not being used anywhere?"
  //
  // The answer is no. See 6.7.5.2(2) of the standard:
  // "
  // if a variable with static storage duration has initialization or a
  // destructor with side effects, it shall not be eliminated even if it appears
  // to be unused, except that a class object or its copy/move may be eliminated
  // asspecified in 11.10.6
  // "
  //
  // You might also be worred that make_property gets called when any
  // member is initialized. Won't our magic just once lambda only get called
  // once, for the first member in any class, and then never again?
  //
  // The answer is again, no. Because this function is a template over
  // MEMBER (which is the member pointer) each member actually gets
  // its own instance of make_property - and each instance of make_property gets
  // it's own instance of _just_once
  static auto _just_once = ([name] {
    get_property_table<class_type>().push_back(
        std::make_unique<property<MEMBER>>(name));

    return '\0';
  })();

  return {};
}
}  // namespace INTERNAL

// this does nothing except for capture the name of the member
// and inject our make_property call. Otherwise, it is entirely
// transparent to the user
#define SERIALIZE(name)                                  \
  name = ::grenewode_serialize::INTERNAL::make_property< \
      &std::decay_t<decltype(*this)>::name>(#name)
}  // namespace grenewode_serialize

#endif
