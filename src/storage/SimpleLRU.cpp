#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) != _lru_index.end()) return Set(key, value); // There is already such a key.
	return PutIfAbsent(key, value);	// There is not such a key.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) != _lru_index.end()) return false; // There is already such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	// Delete obsolete fields until there is free space.
	while (_storage_size + key.size() + value.size() > _max_size)
		Delete(_lru_head->key);
	// Input the new node in a head
	////////////////////////////////
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) == _lru_index.end()) return false; // There is not such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	Delete(key); // Delete the obsolete pair with this key.
	return PutIfAbsent(key, value); // Move new item with this key and value on the top.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
	auto need_iterator = _lru_index.find(key);
	if (need_iterator == _lru_index.end()) return false; // There is not such a key.
	// Delete the pair from the index storage.
	_lru_index.erase(key);
	_storage_size -= (key.size() + need_iterator->second.get().value.size());
	// Delete the pair from the head storage.
	lru_head *current_node = _lru_head.get();
	// Search need node.
	while(current_node != nullptr) {
		if (current_node->key == key)
			break;
		current_node = current_node->next.get();
	}
	///////////////////////////////////////////
}

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
