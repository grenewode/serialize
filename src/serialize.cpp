#include "serialize.hpp"

using namespace grenewode_serialize;

INTERNAL::serializable_registry_type& INTERNAL::serializable_registry() {
  static serializable_registry_type registry;

  return registry;
}