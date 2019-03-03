#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) != _lru_index.end()) return Set(key, value); // There is already such a key.
	return PutIfAbsent(key, value);	// There is not such a key.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) { return false; }

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) == _lru_index.end()) return false; // There is not such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	Delete(key); // Delete the obsolete pair with this key.
	return PutIfAbsent(key, value); // Move new item with this key and value on the top.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { return false; }

// See MapBasedGlobalLockImpl.h
// Do not need "const", as it is necessary to renew the popularity of an item.
bool SimpleLRU::Get(const std::string &key, std::string &value) { // const
	auto need_iterator = _lru_index.find(key);
	if (need_iterator == _lru_index.end()) return false; // There is not such a key.
	value = need_iterator->second.get().value; // There is such an item.
	return Set(key, value);	// Move this item on the top.
}

} // namespace Backend
} // namespace Afina
