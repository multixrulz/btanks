#ifndef BT_ELASTIC_BSET_H__
#define BT_ELASTIC_BSET_H__

#include <cstddef>
#include <stdlib.h>
#include <stdexcept>
#include <vector>
#include <string.h>
#include <assert.h>

template<typename T, unsigned page_size = 1024> 
class elastic_bset {
public: 
	typedef T  key_type;

private: 
	key_type * data;
	size_t size;

public: 
	inline elastic_bset() : data(NULL), size(0) {}
	inline elastic_bset(const elastic_bset<key_type, page_size> &other) : data(NULL), size(0) {
		if (other.data == NULL)
			return;
		data = static_cast<key_type*>(::malloc(other.size * sizeof(key_type)));
		if (data == NULL)
			throw std::runtime_error("malloc failed");
		size = other.size;
		memcpy(data, other.data, size * sizeof(key_type));
	}
	
	static inline size_t align(const size_t size) {
		return (size / page_size + 1) * page_size;
	}
	
	inline ~elastic_bset() {
		::free(data);
	}
	
	inline void resize(size_t new_size) {
		new_size = align(new_size);
		
		if (new_size <= size)
			return;
		
		key_type *p = static_cast<key_type *>(::realloc(data, new_size * sizeof(key_type)));
		if (p == NULL)
			throw std::runtime_error("realloc failed");
		
		::memset(p + size, 0, (new_size - size) * sizeof(key_type));
		
		data = p;
		size = new_size;
	}
	
	inline void insert(const unsigned value) {
		const unsigned offset = value / sizeof(key_type) / 8;
		const unsigned bit = value % ( sizeof(key_type) * 8 );
		resize(offset);
		assert(offset < size);
		data[offset] |= ((key_type)1) << bit;
		//printf("insert bit %u in byte %u\n", bit, offset);
	}
	
	inline void erase(const unsigned &value) {
		const unsigned bit = value % ( sizeof(key_type) * 8 );
		const unsigned offset = value / sizeof(key_type) / 8;
		if (offset >= size)
			return;
		data[offset] &= ~(((key_type)1) << bit);
		//printf("erase bit %u in byte %u\n", bit, offset);
		//add shrinking from head
	}
	
	inline const elastic_bset<key_type, page_size>& operator&= (const elastic_bset<key_type, page_size>& other) {
		size_t msize = size < other.size? size: other.size;
		for(size_t i = 0; i < msize; ++i) {
			data[i] &= other.data[i];
		}
		return *this;
	}

	inline const elastic_bset<key_type, page_size>& operator|= (const elastic_bset<key_type, page_size>& other) {
		resize(other.size);
		size_t msize = size < other.size? size: other.size;
		for(size_t i = 0; i < msize; ++i) {
			data[i] |= other.data[i];
		}
		return *this;
	}

	template<typename K>
	inline void store(std::vector<K> &values) {
		values.clear();
		for(size_t i = 0; i < size; ++i) {
			key_type k = data[i];
			if (k == 0)
				continue;
			for(size_t j = 0; j < sizeof(key_type) * 8; ++j) {
				if ((data[i] & (((key_type)1) << j)) != 0) 
					values.push_back((K)(i * sizeof(key_type) * 8 + j));
			}
		}
	}
};

#endif

