/*************************************************************************
	> File Name: object_pool.h
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Tue 07 Feb 2018 14:38:55 PM CST
 ************************************************************************/
#ifndef OBJDECT_POOL_H
#define OBJDECT_POOL_H
#include<unistd.h>
#include<mutex>
#include<list>
#include<utility>

template<typename T, size_t SIZE>
class ObjectPool {
	public:
		template<typename...Args>
		static ObjectPool* GetInstance(Args...args) {
			if (m_instance == nullptr) {
				std::lock_guard<std::mutex> lk(m_mutex);
				if (m_instance == nullptr) {
					m_instance = new ObjectPool(args...);
				}
			}

			return m_instance;
		}

		T* GetObject(void);
		void ReleaseObject(T* elem);

	private:
		template<typename...Args>
		ObjectPool(Args...args) {
			for (size_t i=0; i<SIZE; ++i) {
				T* elem = new T(std::forward<Args>(args)...);
				m_list.push_back(elem);
			}

			m_len = SIZE;
		}
		~ObjectPool();
    
    		ObjectPool(const ObjectPool& r) = delete;
		ObjectPool& operator=(const ObjectPool& r) = delete;
		ObjectPool(ObjectPool&& r) = delete;
		ObjectPool& operator=(ObjectPool&& r) = delete;
	private:
		size_t m_len;
		static std::mutex m_mutex;
		static std::list<T*> m_list;
		static ObjectPool* m_instance;
};

template<typename T, size_t SIZE>
std::list<T*> ObjectPool<T, SIZE>::m_list = std::list<T*>();

template<typename T, size_t SIZE>
ObjectPool<T, SIZE>* ObjectPool<T, SIZE>::m_instance = nullptr;

template<typename T, size_t SIZE>
std::mutex ObjectPool<T, SIZE>::m_mutex;

template<typename T, size_t SIZE>
T* ObjectPool<T, SIZE>::GetObject(void) {
	std::unique_lock<std::mutex> lk(m_mutex);
	while (m_len == 0) {
		lk.unlock();
		usleep(5000);
		lk.lock();
	}

	T* elem = m_list.front();
	m_list.pop_front();
	--m_len;

	return elem;
}

template<typename T, size_t SIZE>
void ObjectPool<T, SIZE>::ReleaseObject(T* elem) {
	std::lock_guard<std::mutex> lk(m_mutex);
	m_list.push_back(elem);
	++m_len;
  }

template<typename T, size_t SIZE>
ObjectPool<T, SIZE>::~ObjectPool() {
	for (size_t i=0; i<m_len; ++i) {
		std::lock_guard<std::mutex> lk(m_mutex);
		T* elem = m_list.front();
		m_list.pop_front();
		delete elem;
		elem = nullptr;
	}
}

#endif
