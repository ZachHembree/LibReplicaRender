#pragma once
#include <vector>
#include "WeaveUtils/GlobalUtils.hpp"

namespace Weave
{
	/// <summary>
	/// Move-only object pool template
	/// </summary>
	template<typename T>
	class ObjectPool
	{
	public:
		MAKE_MOVE_ONLY(ObjectPool)

		ObjectPool() : objectsOutstanding(0) { }

		/// <summary>
		/// Retrieves and releases ownership of an object from the
		/// pool
		/// </summary>
		virtual T Get()
		{
			objectsOutstanding++;

			if (!objectPool.empty())
			{
				T object = std::move(objectPool.back());
				objectPool.pop_back();
				return object;
			}
			else
			{
				return T();
			}
		}

		/// <summary>
		/// Returns an object to the pool and takes ownership of it
		/// </summary>
		virtual void Return(T&& object)
		{
			objectPool.push_back(std::move(object));
			objectsOutstanding--;

			WV_ASSERT_MSG(objectsOutstanding >= 0, "More objects returned to the pool than issued.");
		}

		/// <summary>
		/// Returns the number of objects currently in use
		/// </summary>
		int GetObjectsOutstanding() const { return objectsOutstanding; }

		/// <summary>
		/// Returns the number of objects available in the pool
		/// </summary>
		int GetObjectsAvailable() const { return (int)objectPool.size(); }

		/// <summary>
		/// Returns the total number of objects tied to the pool, including objects
		/// stored for reuse and objects currently in-use.
		/// </summary>
		int GetTotalObjects() const { return GetObjectsOutstanding() + GetObjectsAvailable(); }

		/// <summary>
		/// Trims the given number of objects from the pool
		/// </summary>
		virtual void TrimPool(size_t count)
		{
			count = std::min(count, objectPool.size());

			while (count > 0)
			{
				objectPool.pop_back();
				count--;
			}
		}

	protected:
		std::vector<T> objectPool;
		int objectsOutstanding;
	};
}