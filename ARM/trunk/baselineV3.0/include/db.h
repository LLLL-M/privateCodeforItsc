_Pragma("once")

#include <cstring>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <type_traits>
#include "rwlock.h"
#include "unqlite.h"

using namespace std;

template<typename T>
struct KV
{
	const void *addr;
	int len;
	KV(const T & t) : addr(&t)
	{
		static_assert(is_pod<T>::value, "The type T isn't pod!");
		len = sizeof(T);
	}
	KV(const void *_addr, int _len) : addr(_addr), len(_len) {}
};
template<>
struct KV<nullptr_t>
{
	const void *addr;
	int len;
	KV(nullptr_t t) : addr(nullptr), len(0) {}
};
template<>
struct KV<char *>
{
	const char * addr;
	int len;
	KV(const char *t) : addr(t)
	{
		len = (t == nullptr) ? 0 : strlen(t);
	}
};
template<>
struct KV<string>
{
	const void * addr;
	int len;
	KV(const string & t) : addr(t.empty() ? nullptr : t.c_str())
	{
		len = t.size();
	}
};
//因为decay无法移除const char *类型的const
template<typename T>
using TypeTraits = typename conditional<is_same<const char *, typename decay<T>::type>::value, char *, typename decay<T>::type>::type;

namespace hik
{
	class database
	{
	private:
		unqlite *db;
		int rc;
		atomic_int times;		//存储、追加、删除操作计数，用来自动提交事务
		const bool uselock;
		hik::rwlock lock;

		static int Consumer(const void * data, unsigned int len, void * user)
		{
			if (user != nullptr)
			{
				string * str = static_cast<string *>(user);
				str->append((const char *)data, len);
			}
			return UNQLITE_OK;
		}

		/*数据存放处理*/
		template<typename K, typename V>
		bool deal(bool store, KV<K> & key, KV<V> & value)
		{
			if (key.addr == nullptr || key.len <= 0 || value.addr == nullptr || value.len <= 0)
				return false;
			if (uselock)
				lock.w_lock();
			if (store)
				rc = unqlite_kv_store(db, key.addr, key.len, value.addr, value.len);
			else
				rc = unqlite_kv_append(db, key.addr, key.len, value.addr, value.len);
			if (uselock)
				lock.w_unlock();
			if (++times > 20)
				commit();
			return (rc == UNQLITE_OK);
		}

		/*数据存放按格式处理*/
		template<typename K, typename ... Args>
		bool dealfmt(bool store, KV<K> & key, const char * fmt, Args&&... args)
		{
			if (key.addr == nullptr || key.len <= 0 || fmt == nullptr)
				return false;
			if (uselock)
				lock.w_lock();
			if (store)
				rc = unqlite_kv_store_fmt(db, key.addr, key.len, fmt, forward<Args>(args)...);
			else
				rc = unqlite_kv_append_fmt(db, key.addr, key.len, fmt, forward<Args>(args)...);
			if (uselock)
				lock.w_unlock();
			if (++times > 20)
				commit();
			return (rc == UNQLITE_OK);
		}

	public:
		database(const char *path = nullptr, bool _uselock = false) : db(nullptr), rc(UNQLITE_OK), uselock(_uselock)
		{
			if (!open(path))
				db = nullptr;
			times = 0;
		}

		~database()
		{
			close();
		}
		
		/*事务操作提交*/
		void commit()
		{
			times = 0;
			unqlite_commit(db);
		}

		/*事务操作回滚*/
		void rollback()
		{
			unqlite_rollback(db);
		}

		/*打开数据库*/
		bool open(const char *path)
		{
			if (db != nullptr)
				return true;
			if (path == nullptr)
				return false;	//path==NULL会创建内存数据库，内存数据库不支持事务操作
			rc = unqlite_open(&db, path, UNQLITE_OPEN_CREATE);
			return (rc == UNQLITE_OK);
		}

		/*关闭数据库*/
		void close()
		{
			if (db != nullptr)
			{
				unqlite_close(db);
				db = nullptr;
			}
		}

		/*数据库键值对存储, k和v的类型可为c字符串、string或者pod类型*/
		template<typename K, typename V>
		bool store(K && k, V && v)
		{
			KV<TypeTraits<K>> key(k);
			KV<TypeTraits<V>> value(v);
			return deal(true, key, value);
		}

		template<typename K>
		bool store(K && k, const void * data, int datalen)
		{
			KV<TypeTraits<K>> key(k);
			KV<TypeTraits<void *>> value(data, datalen);
			return deal(true, key, value);
		}

		template<typename V>
		bool store(const void * k, int keylen, V && v)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			KV<TypeTraits<V>> value(v);
			return deal(true, key, value);
		}

		bool store(const void * k, int keylen, const void * data, int datalen)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			KV<TypeTraits<void *>> value(data, datalen);
			return deal(true, key, value);
		}

		template<typename K, typename ... Args>
		bool storefmt(K && k, const char * fmt, Args&&... args)
		{
			KV<TypeTraits<K>> key(k);
			return dealfmt(true, key, fmt, forward<Args>(args)...);
		}

		template<typename ... Args>
		bool storefmt(const void * k, int keylen, const char * fmt, Args&&... args)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			return dealfmt(true, key, fmt, forward<Args>(args)...);
		}

		/*数据库键值追加, k和v的类型可为c字符串、string或者pod类型*/
		template<typename K, typename V>
		bool append(K && k, V && v)
		{
			KV<TypeTraits<K>> key(k);
			KV<TypeTraits<V>> value(v);
			return deal(false, key, value);
		}

		template<typename K>
		bool append(K && k, const void * data, int datalen)
		{
			KV<TypeTraits<K>> key(k);
			KV<TypeTraits<void *>> value(data, datalen);
			return deal(false, key, value);
		}

		template<typename V>
		bool append(const void * k, int keylen, V && v)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			KV<TypeTraits<V>> value(v);
			return deal(false, key, value);
		}

		bool append(const void * k, int keylen, const void * data, int datalen)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			KV<TypeTraits<void *>> value(data, datalen);
			return deal(false, key, value);
		}

		template<typename K, typename ... Args>
		bool appendfmt(K && k, const char * fmt, Args&&... args)
		{
			KV<TypeTraits<K>> key(k);
			return dealfmt(false, key, fmt, forward<Args>(args)...);
		}

		template<typename ... Args>
		bool appendfmt(const void * k, int keylen, const char * fmt, Args&&... args)
		{
			KV<TypeTraits<void *>> key(k, keylen);
			return dealfmt(false, key, fmt, forward<Args>(args)...);
		}

		/*获取键值, k的类型可为c字符串、string或者pod类型*/
		template<typename K>
		string fetch(K && k)
		{
			KV<TypeTraits<K>> key(k);
			return fetch(key.addr, key.len);
		}

		string fetch(const void * key, int keylen)
		{
			string data;
			if (key != nullptr && keylen > 0)
			{
				if (uselock)
					lock.r_lock();
				rc = unqlite_kv_fetch_callback(db, key, keylen, Consumer, &data);
				if (uselock)
					lock.r_unlock();
			}
			else
				rc = UNQLITE_INVALID;
			return data;
		}

		template<typename K>
		bool fetch(K && k, void * buf, size_t * size)
		{
			KV<TypeTraits<K>> key(k);
			return fetch(key.addr, key.len, buf, size);
		}

		bool fetch(const void * key, int keylen, void * buf, size_t * size)
		{
			if (key != nullptr && keylen > 0 && buf != nullptr && size != nullptr && *size > 0)
			{
				unqlite_int64 _size = *size;
				if (uselock)
					lock.r_lock();
				rc = unqlite_kv_fetch(db, key, keylen, buf, &_size);
				if (uselock)
					lock.r_unlock();
				*size = (size_t)_size;
			}
			else
				rc = UNQLITE_INVALID;
			return (rc == UNQLITE_OK);
		}

		/*按指定的条件获取，若不指定获取条件则默认全部获取*/
		vector<pair<string/*key*/, string/*data*/>> fetch_if(function<bool(const string &/*key*/)> condition = nullptr)
		{
			vector<pair<string, string>> vec;
			unqlite_kv_cursor *pCursor;
			
			if (unqlite_kv_cursor_init(db, &pCursor) != UNQLITE_OK)
				return vec;
			/*轮询记录进行获取*/
			string key, data;
			if (uselock)
				lock.r_lock();
			for (unqlite_kv_cursor_first_entry(pCursor); unqlite_kv_cursor_valid_entry(pCursor); unqlite_kv_cursor_next_entry(pCursor))
			{
			    unqlite_kv_cursor_key_callback(pCursor, Consumer, &key);
			    if (condition == nullptr || condition(key))
			    {
			    	unqlite_kv_cursor_data_callback(pCursor, Consumer, &data);
			    	vec.emplace_back(key, data);
			    }
			    key.clear();
			    data.clear();	//此处必须调用clear函数，因为在Consumer函数里面是以append的方式追加字符串
			}
			if (uselock)
				lock.r_unlock();
			unqlite_kv_cursor_release(db, pCursor);
			return vec;
		}

		/*按指定键删除, k的类型可为c字符串、string或者pod类型*/
		template<typename K>
		void del(K && k)
		{
			KV<TypeTraits<K>> key(k);
			del(key.addr, key.len);
		}

		void del(const void * key, int keylen)
		{
			if (key == nullptr || keylen <= 0)
				return;
			if (uselock)
				lock.w_lock();
			unqlite_kv_delete(db, key, keylen);
			if (uselock)
				lock.w_unlock();
			if (++times > 20)
				commit();
		}

		/*按指定的条件删除，若不指定删除条件则默认全部删除*/
		void del_if(function<bool(const string &/*key*/)> condition = nullptr)
		{
			unqlite_kv_cursor *pCursor;
			if (unqlite_kv_cursor_init(db, &pCursor) != UNQLITE_OK)
				return;

			string key;
			/*轮询记录进行删除*/
			if (uselock)
				lock.w_lock();
			for (unqlite_kv_cursor_first_entry(pCursor); unqlite_kv_cursor_valid_entry(pCursor); unqlite_kv_cursor_next_entry(pCursor))
			{
			    unqlite_kv_cursor_key_callback(pCursor, Consumer, &key);
			    if (condition == nullptr || condition(key)){
			    	//unqlite_kv_cursor_delete_entry(pCursor); 此接口有问题，删除之后当前游标会自动后移，但是移动64个之后便不再移动了
			    	unqlite_kv_delete(db, key.data(), key.size());
			    }
			    key.clear();
			}
			if (uselock)
				lock.w_unlock();
			unqlite_kv_cursor_release(db, pCursor);
		}

		/*数据库存储的记录个数*/
		size_t count()
		{
			size_t count = 0;
			unqlite_kv_cursor *pCursor;
			if (unqlite_kv_cursor_init(db, &pCursor) != UNQLITE_OK)
				return 0;
			/*轮询记录进行获取*/
			if (uselock)
				lock.r_lock();
			for (unqlite_kv_cursor_first_entry(pCursor); unqlite_kv_cursor_valid_entry(pCursor); unqlite_kv_cursor_next_entry(pCursor))
				count++;
			if (uselock)
				lock.r_unlock();
			return count;
		}

		/*获取数据库操作的错误信息*/
		string geterrlog()
		{
			string err;
			if (rc != UNQLITE_OK)
			{
				const char *zBuf;
		    	int iLen = 0;
		    	unqlite_config(db, UNQLITE_CONFIG_ERR_LOG, &zBuf, &iLen);
		    	if (iLen > 0)
		    		err.assign(zBuf, iLen);
		    	if(rc != UNQLITE_BUSY && rc != UNQLITE_NOTIMPLEMENTED)
		    		unqlite_rollback(db);
			}
			return err;
		}
	#if 0 //以下两个函数作用不大，所以注释掉了
		/*配置生成hash值的函数*/
		void ConfigHashFunc(unsigned int (*xHash)(const void * key, unsigned int keylen))
		{
			unqlite_kv_config(db, UNQLITE_KV_CONFIG_HASH_FUNC, xHash);
		}

		/*配置key的比较函数*/
		void ConfigCmpFunc(int (*xCmp)(const void * userkey, const void * recordkey, unsigned int len))
		{
			unqlite_kv_config(db, UNQLITE_KV_CONFIG_CMP_FUNC, xCmp);
		}
	#endif
	protected:
	};
}
