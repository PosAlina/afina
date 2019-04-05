#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) {
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	auto need_iterator = _lru_index.find(key);
	if (need_iterator != _lru_index.end()) return UpdateNode(value, &(need_iterator->second.get())); // There is already such a key.
	return PutNewNode(key, value);	// There is not such a key.
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
	if (_lru_index.find(key) != _lru_index.end()) return false; // There is already such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	return PutNewNode(key, value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) {
	auto need_iterator = _lru_index.find(key);
	if (need_iterator == _lru_index.end()) return false; // There is not such a key.
	if ((key.size() + value.size()) > _max_size) return false; // This pair does not fit in the cache.
	return UpdateNode(value, &(need_iterator->second.get()));
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) {
	auto need_iterator = _lru_index.find(key);
	if (need_iterator == _lru_index.end()) return false; // There is not such a key. 
	// Delete the pair from the index storage.
	_lru_index.erase(key);
	_storage_size -= (key.size() + need_iterator->second.get().value.size());
	// Delete the pair from the head storage.
	lru_node *current_node = _lru_head.get();
	// Search need node.
	while(current_node != nullptr) {
		if (current_node->key == key) break;
		current_node = current_node->next.get();
	}
	if (current_node->next != nullptr) { // It is not the last node.
		//Change prev node.
		if (current_node->prev == nullptr) {
			_lru_head = std::move(current_node->next); // It is first node.
			current_node->next->prev = nullptr;
		}
		else {
			current_node->next->prev = current_node->prev;
			current_node->prev->next = std::move(current_node->next); //It is not first node.
		}
	}
	else { // It is the last node.
		if (_lru_last_node->prev == nullptr) { // There is one element in the storage.
			_lru_head.reset();
			_lru_last_node = nullptr;
		}
		else {
			_lru_last_node = _lru_last_node->prev;
			_lru_last_node->next.reset();
		}
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
// Do not need "const", as it is necessary to renew the popularity of an item.
bool SimpleLRU::Get(const std::string &key, std::string &value) { // const
	auto need_iterator = _lru_index.find(key);
	if (need_iterator == _lru_index.end()) return false; // There is not such a key.
	value = need_iterator->second.get().value; // There is such an item.
	return MoveNode(&(need_iterator->second.get()));	// Move this item on the top.
}

//Auxiliary methods
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::DeleteLastNode() {
	// Delete the pair from the index storage.
	_lru_index.erase(_lru_last_node->key);
	_storage_size -= ((_lru_last_node->key).size() + (_lru_last_node->value).size());
	if (_lru_last_node->prev == nullptr) { // There is one element in the storage.
		_lru_head.reset();
		_lru_last_node = nullptr;
	}
	else {
		_lru_last_node = _lru_last_node->prev;
		_lru_last_node->next.reset();
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutNewNode(const std::string &key, const std::string &value) {
	// Delete obsolete fields until there is free space.
	while (_storage_size + key.size() + value.size() > _max_size)
		DeleteLastNode();
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
	// Input the new node in the index storage.
	_lru_index.insert({std::reference_wrapper<const std::string>(current_node->key), std::reference_wrapper<lru_node>(*current_node)});
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::MoveNode(lru_node *need_node) {
	if (need_node->prev != nullptr) { // This node in head already
		_lru_head->prev = need_node;
		if (need_node->next != nullptr) { // This node in the tail
			_lru_last_node = need_node->prev;
			_lru_last_node->next = nullptr;
		}
		else need_node->next->prev = std::move(need->prev); // This node in the middle of the query
		need_node->prev->next = std::move(need_node->next);
		need_node->prev = nullptr;
		need_node->next = std::move(_lru_head);
	}
	return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::UpdateNode(const std::string &value, lru_node *need_node) {
	MoveNode(need_node);
	// Delete obsolete fields until there is free space.
	while ((_storage_size + value.size() - (need_node->value).size()) > _max_size)
		DeleteLastNode();
	_storage_size += value.size() - (need_node->value).size()e;
	need_node->value = value;
	return true;
}
} // namespace Backend
} // namespace Afina
