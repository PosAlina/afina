#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

bool SimpleLRU::PutNewField(const std::string &key, const std::string &value) {
	// Delete obsolete fields until there is free space.
	while (_storage_size + key.size() + value.size() > _max_size)
		Delete(_lru_last_node->key);
	// Input the new node in the index storage.
	_lru_index.insert({std::reference_wrapper<const std::string>(current_node->key), std::reference_wrapper<lru_node>(*current_node)});
	_storage_size += key.size() + value.size();
	// Input the new node in a head
	lru_node *current_node;
	if (_lru_head != nullptr) { // There are elements in the storage.
		current_node = new lru_node{key, value, nullptr, std::move(_lru_head)};
		_lru_head.reset(current_node);
		_lru_head->next->prev = current_node;
	}
	else { // There are not elements in the storage.
		current_node = new lru_node{key, value, nullptr, std::unique_ptr<lru_node>()};
		_lru_head.reset(current_node);
		_lru_last_node = current_node;
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.	
	if (_lru_index.find(key) != _lru_index.end()) return Set(key, value); // There is already such a key.
	return PutNewField(key, value);	// There is not such a key.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) != _lru_index.end()) return false; // There is already such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	return PutNewField(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) == _lru_index.end()) return false; // There is not such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	Delete(key);
	return PutIfAbsent(key, value);
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
	if (current_node->next != nullptr) { // It is not the last node.
		//Change prev node.
		if (current_node->prev == nullptr) head = std::move(current_node->next); // It is first node.
		else current_node->prev->next = std::move(currrent_node->next); //It is not first node.
		current_node->next->prev = current_node->prev;
	}
	else { // It is the last node.
		if (_lru_last_node->prev == nullptr) _lru_head.reset(); // There is one element in the storage.
		else _lru_last_node->prev->next.reset();
		_lru_last_node = _lru_last_node->prev;	
	}
	return true;
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
