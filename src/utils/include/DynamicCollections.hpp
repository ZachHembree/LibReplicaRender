#ifndef DynamicCollections
#define DynamicCollections

#include<vector>
#include<exception>
#include<string>

namespace Replica
{
	template<typename T> class DynamicArrayBase;
	template<typename T> class DynamicArray;
	template<typename T> class UniqueArray;

	/// <summary>
	/// Interface template for dynamic arrays/vectors
	/// </summary>
	template<typename T>
	class IDynamicCollection
	{
	public:
		/// <summary>
		/// Returns the length of the array.
		/// </summary>
		virtual size_t GetLength() const = 0;

		/// <summary>
		/// Returns the size of the array in bytes
		/// </summary>
		virtual size_t GetSize() const = 0;

		/// <summary>
		/// Provides indexed access to array member references.
		/// </summary>
		virtual T& operator[](size_t index) = 0;

		/// <summary>
		/// Provides indexed access to array members using constant references.
		/// </summary>
		virtual const T& operator[](size_t index) const = 0;

		/// <summary>
		/// Returns a copy of the pointer to the backing the array.
		/// </summary>
		virtual T* GetPtr() = 0;

		/// <summary>
		/// Returns a const copy of the pointer to the backing the array.
		/// </summary>
		virtual const T* GetPtr() const = 0;
	};

	/// <summary>
	/// Dynamically allocated array with members of type T and a fixed length.
	/// </summary>
	template<typename T>
	class DynamicArrayBase : public IDynamicCollection<T>
	{
	protected:
		/// <summary>
		/// Array length
		/// </summary>
		size_t length;

		/// <summary>
		/// Array pointer
		/// </summary>
		T* data;

		/// <summary>
		/// Initializes an empty array with a null pointer.
		/// </summary>
		DynamicArrayBase() :
			length(0),
			data(nullptr)
		{ }

		/// <summary>
		/// Initializes a new dynamic array using an initializer list.
		/// </summary>
		DynamicArrayBase(const std::initializer_list<T>& list) noexcept :
			length(list.size()),
			data(new T[length])
		{
			std::copy(list.begin(), list.end(), data);
		}

		/// <summary>
		/// Initializes a dynamic array with the given length.
		/// </summary>
		DynamicArrayBase(size_t length) :
			length(length),
			data((length > 0) ? new T[length] : nullptr)
		{ }

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Creates a copy.
		/// </summary>
		DynamicArrayBase(const T* data, const size_t length) :
			length(new T[length]),
			data(data)
		{ 
			memcpy(this->data, data, length * sizeof(T));
		}

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Takes ownership of the pointer.
		/// </summary>
		DynamicArrayBase(T*&& data, const size_t length) :
			length(length),
			data(data)
		{ }

		/// <summary>
		/// Initializes a copy of the given dynamic array.
		/// </summary>
		DynamicArrayBase(const DynamicArrayBase& rhs) :
			length(rhs.length),
			data(new T[rhs.length])
		{
			memcpy(data, rhs.data, length * sizeof(T));
		}

		/// <summary>
		/// Initializes a new dynamic array by moving the contents of the given array to itself.
		/// </summary>
		DynamicArrayBase(DynamicArrayBase&& rhs) noexcept :
			length(rhs.length),
			data(rhs.data)
		{
			rhs.data = nullptr;
			rhs.length = -1;
		}

		/// <summary>
		/// Copy assignment operator.
		/// </summary>
		DynamicArrayBase& operator=(const DynamicArrayBase& rhs)
		{
			if (this != &rhs)
			{
				if (length != rhs.length)
				{ 
					delete[] data;
					data = new T[rhs.length];
				}

				length = rhs.length;
				memcpy(data, rhs.data, length * sizeof(T));
			}

			return *this;
		}

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		DynamicArrayBase& operator=(DynamicArrayBase&& rhs) noexcept
		{
			delete[] data;

			data = rhs.data;
			length = rhs.length;
			rhs.data = nullptr;
			rhs.length = 0;

			return *this;
		}

	public:
		/// <summary>
		/// Deallocates the memory backing the array.
		/// </summary>
		virtual ~DynamicArrayBase()
		{
			delete[] data;
		}

		/// <summary>
		/// Returns the length of the array.
		/// </summary>
		size_t GetLength() const override { return length; }

		/// <summary>
		/// Returns the size of the array in bytes
		/// </summary>
		size_t GetSize() const override { return length * sizeof(T); }

		/// <summary>
		/// Provides indexed access to array member references.
		/// </summary>
		T& operator[](size_t index) override
		{
	#if _CONTAINER_DEBUG_LEVEL > 0
			if (index >= length)
			{
				char buffer[100];
				sprintf_s(buffer, 100, "Array index out of range. Index: %tu, Length %tu", index, length);

				throw std::exception(buffer);
			}
	#endif

			return data[index];
		}

		/// <summary>
		/// Provides indexed access to array members using constant references.
		/// </summary>
		const T& operator[](size_t index) const override
		{
	#if _CONTAINER_DEBUG_LEVEL > 0
			if (index >= length)
			{
				char buffer[100];
				sprintf_s(buffer, 100, "Array index out of range. Index: %tu, Length %tu", index, length);

				throw std::exception(buffer);
			}
	#endif

			return data[index];
		}

		/// <summary>
		/// Returns a copy of the pointer to the backing the array.
		/// </summary>
		T* GetPtr() override { return data; }

		/// <summary>
		/// Returns a const copy of the pointer to the backing the array.
		/// </summary>
		const T* GetPtr() const override { return data; }
	};

	/// <summary>
	/// Dynamically allocated array with members of type T and a fixed length.
	/// </summary>
	template<typename T>
	class DynamicArray : public DynamicArrayBase<T>
	{
	public:
		/// <summary>
		/// Initializes an empty array with a null pointer.
		/// </summary>
		DynamicArray() : DynamicArrayBase<T>() { }

		/// <summary>
		/// Initializes a new dynamic array from an initializer list.
		/// </summary>
		DynamicArray(const std::initializer_list<T>& initializerList) noexcept : 
			DynamicArrayBase<T>(initializerList)
		{ }

		/// <summary>
		/// Initializes a dynamic array with the given length.
		/// </summary>
		DynamicArray(size_t length) : 
			DynamicArrayBase<T>(length) 
		{ }

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Creates a copy.
		/// </summary>
		DynamicArray(const T* data, const size_t length) :
			DynamicArrayBase<T>(data, length)
		{ }

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Takes ownership of the pointer.
		/// </summary>
		DynamicArray(T*&& data, const size_t length) :
			DynamicArrayBase<T>(std::move(data), length) 
		{ }

		/// <summary>
		/// Initializes a copy of the given dynamic array.
		/// </summary>
		DynamicArray(const DynamicArray& rhs) : 
			DynamicArrayBase<T>(rhs) 
		{ }

		/// <summary>
		/// Initializes a copy of the given dynamic array.
		/// </summary>
		explicit DynamicArray(const UniqueArray<T>& rhs) : 
			DynamicArrayBase<T>((DynamicArrayBase<T>&)rhs)
		{ }

		/// <summary>
		/// Initializes a new dynamic array by moving the contents of the given array to itself.
		/// </summary>
		DynamicArray(DynamicArray&& rhs) noexcept : 
			DynamicArrayBase<T>(std::move(rhs)) 
		{ }

		/// <summary>
		/// Initializes a new dynamic array by moving the contents of the given unique array to itself.
		/// </summary>
		explicit DynamicArray(UniqueArray<T>&& rhs) noexcept : 
			DynamicArrayBase<T>((DynamicArrayBase<T>&&)(rhs))
		{ }

		/// <summary>
		/// Copy assignment operator.
		/// </summary>
		DynamicArray& operator=(const DynamicArray& rhs)
		{ return static_cast<DynamicArray&>(DynamicArrayBase<T>::operator=(rhs)); }

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		DynamicArray& operator=(DynamicArray&& rhs) noexcept 
		{ return static_cast<DynamicArray&>(DynamicArrayBase<T>::operator=(std::move(rhs))); }
	};

	/// <summary>
	/// Unique dynamically allocated array. Implicit copy assignment disallowed.
	/// </summary>
	template<typename T>
	class UniqueArray : public DynamicArrayBase<T>
	{
	private:
		/// <summary>
		/// Initializes a copy of the given unique array.
		/// </summary>
		UniqueArray(const UniqueArray& rhs) : 
			DynamicArrayBase<T>(rhs) 
		{ }

	public:
		/// <summary>
		/// Initializes an empty array with a null pointer.
		/// </summary>
		UniqueArray() : 
			DynamicArrayBase<T>()
		{ }

		/// <summary>
		/// Initializes a new unique array from an initializer list.
		/// </summary>
		UniqueArray(const std::initializer_list<T>& initializerList) noexcept :
			DynamicArrayBase<T>(initializerList)
		{ }

		/// <summary>
		/// Initializes a unique array with the given length.
		/// </summary>
		UniqueArray(size_t length) :
			DynamicArrayBase<T>(length) 
		{ }

		/// <summary>
		/// Initializes a new unique array object using the given pointer and length.
		/// Creates a copy.
		/// </summary>
		UniqueArray(const T* data, const size_t length) :
			DynamicArrayBase<T>(data, length)
		{ }

		/// <summary>
		/// Creates a new unique array using the given pointer and length.
		/// Takes ownership of the pointer.
		/// </summary>
		UniqueArray(T*&& data, const size_t length) :
			DynamicArrayBase<T>(std::move(data), length) 
		{ }

		/// <summary>
		/// Initializes a copy of the given dynamic array.
		/// </summary>
		explicit UniqueArray(const DynamicArray<T>& rhs) : 
			DynamicArrayBase<T>((DynamicArrayBase<T>&)rhs)
		{ }

		/// <summary>
		/// Initializes a new unique array by moving the contents of the given unique array to itself.
		/// </summary>
		UniqueArray(UniqueArray&& rhs) noexcept : 
			DynamicArrayBase<T>(std::move(rhs)) 
		{ }

		/// <summary>
		/// Initializes a new unique array by moving the contents of the given dynamic array to itself.
		/// </summary>
		explicit UniqueArray(DynamicArray<T>&& rhs) noexcept :
			DynamicArrayBase<T>((DynamicArrayBase<T>&&)rhs)
		{ }
	
		/// <summary>
		/// Copy assignment operator. Disabled.
		/// </summary>
		UniqueArray& operator=(const UniqueArray& rhs) = delete;

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		UniqueArray& operator=(UniqueArray&& rhs) noexcept 
		{ return static_cast<UniqueArray&>(DynamicArrayBase<T>::operator=(std::move(rhs))); }

		/// <summary>
		/// Returns a new copy of the array.
		/// </summary>
		UniqueArray GetCopy() 
		{ return UniqueArray(*this); }
	};

	/// <summary>
	/// Vector{T} implementing IDynamicCollection{T}
	/// </summary>
	template<typename T>
	class Vector : public IDynamicCollection<T>, private std::vector<T>
	{
	public:
		using std::vector<T>::push_back;
		using std::vector<T>::emplace_back;
		using std::vector<T>::pop_back;
		using std::vector<T>::begin;
		using std::vector<T>::end;
		using std::vector<T>::erase;
		using std::vector<T>::max_size;
		using std::vector<T>::resize;
		using std::vector<T>::capacity;
		using std::vector<T>::empty;
		using std::vector<T>::reserve;
		using std::vector<T>::shrink_to_fit;

		/// <summary>
		/// Returns the length of the vector.
		/// </summary>
		size_t GetLength() const override { return this->size(); }

		/// <summary>
		/// Returns the size of the vector in bytes
		/// </summary>
		size_t GetSize() const override { return this->size() * sizeof(T); }

		/// <summary>
		/// Returns a copy of the pointer to the backing the vector.
		/// </summary>
		T* GetPtr() override { return this->data(); }

		/// <summary>
		/// Returns a const copy of the pointer to the backing the vector.
		/// </summary>
		const T* GetPtr() const override { return this->data(); }

		/// <summary>
		/// Provides indexed access to vector member references.
		/// </summary>
		T& operator[](size_t index) override { return this->at(index); }

		/// <summary>
		/// Provides indexed access to vector members using constant references.
		/// </summary>
		const T& operator[](size_t index) const override { return this->at(index); }
	};

	/// <summary>
	/// Move-only wrapper for Replica::Vector(T)
	/// </summary>
	template<typename T>
	class UniqueVector : public Vector<T>
	{
	private:
		/// <summary>
		/// Initializes a new copy of the given unique vector.
		/// </summary>
		UniqueVector(const UniqueVector& rhs) : 
			Vector<T>(rhs)
		{ };

		/// <summary>
		/// Copy assignment operator. Disabled.
		/// </summary>
		UniqueVector& operator=(const UniqueVector& rhs) = delete;

	public:
		/// <summary>
		/// Initializes a new unique vector.
		/// </summary>
		UniqueVector() : 
			Vector<T>() 
		{ }

		/// <summary>
		/// Initializes a new unique vector from an initializer list.
		/// </summary>
		UniqueVector(const std::initializer_list<T>& initializerList) noexcept :
			Vector<T>(initializerList)
		{ }

		/// <summary>
		/// Initializes a new unique vector with the given capacity.
		/// </summary>
		explicit UniqueVector(size_t capacity) : 
			Vector<T>(capacity)
		{ }

		/// <summary>
		/// Initializes a new vector by moving the contents the given vector into itself.
		/// </summary>
		UniqueVector(UniqueVector&& rhs) noexcept :
			Vector<T>(std::move(rhs))
		{ }

		/// <summary>
		/// Initializes a new unique vector by moving the contents the given Vector into itself.
		/// </summary>
		explicit UniqueVector(Vector<T>&& rhs) noexcept :
			Vector<T>(std::move(rhs))
		{ }

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		UniqueVector& operator=(UniqueVector&& rhs) noexcept
		{ return static_cast<UniqueVector&>(Vector<T>::operator=(std::move(rhs))); }

		/// <summary>
		/// Returns a new copy of the unique vector.
		/// </summary>
		UniqueVector GetCopy() 
		{ return UniqueVector(*this); }
	};

}
#endif