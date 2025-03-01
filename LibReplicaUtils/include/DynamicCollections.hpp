#ifndef DynamicCollections
#define DynamicCollections

#include <vector>
#include <exception>
#include <string>
#include <iterator>
#include <memory>

namespace Replica
{
	template<typename T> class DynamicArrayBase;
	template<typename T> class DynamicArray;
	template<typename T> class UniqueArray;

	/// <summary>
	/// Basic iterator template for contiguous collections
	/// </summary>
	template<typename ValueType>
	class DynIterator
	{
	public:
		using iterator_category = std::contiguous_iterator_tag;
		using value_type = ValueType;
		using difference_type = ptrdiff_t;
		using pointer = ValueType*;
		using reference = ValueType&;

		DynIterator() : pData(nullptr) { }

		template <typename T = ValueType, typename = std::enable_if_t<!std::is_const<T>::value>>
		DynIterator(T* pData) : pData(pData) { }

		DynIterator(const ValueType* pData) : pData(const_cast<ValueType*>(pData)) { }

		// Dereference operators
		reference operator*() const { return *pData; }
		pointer operator->() const { return pData; }

		// Random access element access
		reference operator[](difference_type offset) const { return *(pData + offset); }

		// Arithmetic operators
		DynIterator& operator++() { ++pData; return *this; }
		DynIterator operator++(int) { DynIterator temp = *this; ++(*this); return temp; }

		DynIterator& operator--() { --pData; return *this; }
		DynIterator operator--(int) { DynIterator temp = *this; --(*this); return temp; }

		DynIterator operator+(difference_type offset) const { return DynIterator(pData + offset); }
		DynIterator operator-(difference_type offset) const { return DynIterator(pData - offset); }

		DynIterator& operator+=(difference_type offset) { pData += offset; return *this; }
		DynIterator& operator-=(difference_type offset) { pData -= offset; return *this; }

		difference_type operator-(const DynIterator& other) const { return pData - other.pData; }

		// Comparison operators
		bool operator==(const DynIterator& other) const { return pData == other.pData; }
		bool operator!=(const DynIterator& other) const { return pData != other.pData; }
		bool operator<(const DynIterator& other) const { return pData < other.pData; }
		bool operator>(const DynIterator& other) const { return pData > other.pData; }
		bool operator<=(const DynIterator& other) const { return pData <= other.pData; }
		bool operator>=(const DynIterator& other) const { return pData >= other.pData; }

	protected:
		pointer pData;
	};

	/// <summary>
	/// Interface template for dynamic arrays/vectors
	/// </summary>
	template<typename T>
	class IDynamicArray
	{
	public:
		using Iterator = DynIterator<T>;
		using RevIterator = std::reverse_iterator<DynIterator<T>>;

		/// <summary>
		/// Returns the length of the array.
		/// </summary>
		virtual size_t GetLength() const = 0;

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

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		virtual Iterator begin() = 0;

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		virtual Iterator end() = 0;

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		virtual const Iterator begin() const = 0;

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		virtual const Iterator end() const = 0;

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		virtual RevIterator rbegin() { return RevIterator(begin()); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		virtual RevIterator rend() { return RevIterator(end()); }

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		virtual const RevIterator rbegin() const { return RevIterator(begin()); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		virtual const RevIterator rend() const { return RevIterator(end()); }
	};

	/// <summary>
	/// Returns the size of the array in bytes
	/// </summary>
	template<typename T>
	size_t GetArrSize(const IDynamicArray<T>& arr) { return arr.GetLength() * sizeof(T); }

	/// <summary>
	/// Returns the size of the array in bytes
	/// </summary>
	template<typename T>
	size_t GetArrSize(const std::vector<T>& arr) { return arr.size() * sizeof(T); }

	/// <summary>
	/// Dynamically allocated array with members of type T and a fixed length.
	/// </summary>
	template<typename T>
	class DynamicArrayBase : public IDynamicArray<T>
	{
	public:
		using Iterator = IDynamicArray<T>::Iterator;
		using RevIterator = IDynamicArray<T>::RevIterator;

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
		explicit DynamicArrayBase(size_t length) :
			length(length),
			data((length > 0) ? new T[length]() : nullptr)
		{ }

		/// <summary>
		/// Initializes a dynamic array with the given length.
		/// </summary>
		DynamicArrayBase(size_t length, const T& initValue) :
			length(length),
			data((length > 0) ? new T[length](initValue) : nullptr)
		{ }

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Creates a copy.
		/// </summary>
		DynamicArrayBase(const T* data, const size_t length) :
			length(length),
			data(new T[length])
		{ 
			std::copy(data, data + length, this->data);
		}

		/// <summary>
		/// Initializes a new dynamic array object using the given pointer and length.
		/// Takes ownership of the pointer.
		/// </summary>
		DynamicArrayBase(std::unique_ptr<T[]>&& ptr, size_t length)
			: length(length), data(ptr.release())
		{ }

		/// <summary>
		/// Initializes a copy of the given dynamic array.
		/// </summary>
		DynamicArrayBase(const DynamicArrayBase& rhs) :
			length(rhs.length),
			data(new T[rhs.length])
		{
			std::copy(rhs.data, rhs.data + length, this->data);
		}

		/// <summary>
		/// Initializes a new dynamic array by moving the contents of the given array to itself.
		/// </summary>
		DynamicArrayBase(DynamicArrayBase&& rhs) noexcept :
			length(rhs.length),
			data(rhs.data)
		{
			rhs.data = nullptr;
			rhs.length = 0;
		}

	public:
		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		Iterator begin() override { return Iterator(data); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		Iterator end() override { return Iterator(data + length); }

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		const Iterator begin() const override { return Iterator(data); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		const Iterator end() const override { return Iterator(data + length); }

		/// <summary>
		/// Copy assignment operator.
		/// </summary>
		DynamicArrayBase& operator=(const DynamicArrayBase& rhs)
		{
			if (this != &rhs)
			{
				if (length != rhs.length)
				{ 
					if (data != nullptr)
						delete[] data;

					data = new T[rhs.length];
				}

				length = rhs.length;
				std::copy(rhs.data, rhs.data + length, data);
			}

			return *this;
		}

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		DynamicArrayBase& operator=(DynamicArrayBase&& rhs) noexcept
		{
			if (data != nullptr)
				delete[] data;

			data = rhs.data;
			length = rhs.length;
			rhs.data = nullptr;
			rhs.length = 0;

			return *this;
		}

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
		explicit DynamicArray(size_t length) : 
			DynamicArrayBase<T>(length) 
		{ }

		/// <summary>
		/// Initializes a dynamic array with the given length.
		/// </summary>
		DynamicArray(size_t length, const T& initValue) :
			DynamicArrayBase<T>(length, initValue)
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
		DynamicArray(std::unique_ptr<T[]>&& ptr, size_t length) :
			DynamicArrayBase<T>(std::move(ptr), length) 
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
		{
			DynamicArrayBase<T>::operator=(rhs); 
			return *this;
		}

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		DynamicArray& operator=(DynamicArray&& rhs) noexcept 
		{
			DynamicArrayBase<T>::operator=(std::move(rhs)); 
			return *this;
		}
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
		explicit UniqueArray(size_t length) :
			DynamicArrayBase<T>(length) 
		{ }

		/// <summary>
		/// Initializes a dynamic array with the given length.
		/// </summary>
		UniqueArray(size_t length, const T& initValue) :
			DynamicArrayBase<T>(length, initValue)
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
		UniqueArray(std::unique_ptr<T[]>&& ptr, size_t length) :
			DynamicArrayBase<T>(std::move(ptr), length) 
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
		{ 
			DynamicArrayBase<T>::operator=(std::move(rhs)); 
			return *this;
		}

		/// <summary>
		/// Returns a new copy of the array.
		/// </summary>
		UniqueArray GetCopy() const
		{ return UniqueArray(*this); }
	};

	/// <summary>
	/// Vector{T} implementing IDynamicCollection{T}
	/// </summary>
	template<typename T>
	class Vector : public IDynamicArray<T>, private std::vector<T>
	{
	public:
		using Iterator = IDynamicArray<T>::Iterator;
		using RevIterator = IDynamicArray<T>::RevIterator;

		using std::vector<T>::push_back;
		using std::vector<T>::emplace_back;
		using std::vector<T>::pop_back;
		using std::vector<T>::clear;
		using std::vector<T>::max_size;
		using std::vector<T>::resize;
		using std::vector<T>::capacity;
		using std::vector<T>::reserve;
		using std::vector<T>::empty;
		using std::vector<T>::shrink_to_fit;

		Vector(const IDynamicArray<T>& other) :
			std::vector<T>(other.GetPtr(), other.GetPtr() + other.GetLength())
		{ }

		/// <summary>
		/// Initializes a new unique vector.
		/// </summary>
		Vector() : std::vector<T>() { }

		/// <summary>
		/// Initializes a new unique vector from an initializer list.
		/// </summary>
		Vector(const std::initializer_list<T>& initializerList) noexcept :
			std::vector<T>(initializerList)
		{ }

		/// <summary>
		/// Initializes a new unique vector with the given capacity.
		/// </summary>
		explicit Vector(size_t capacity)
		{
			this->reserve(capacity);
		}

		/// <summary>
		/// Initializes a new vector by copying the contents the given vector into itself.
		/// </summary>
		Vector(const Vector& rhs) noexcept : std::vector<T>(rhs) { }

		/// <summary>
		/// Initializes a new vector by moving the contents the given vector into itself.
		/// </summary>
		Vector(Vector&& rhs) noexcept : std::vector<T>(std::move(rhs)) { }

		/// <summary>
		/// Adds a copy of the given value to the end of the vector
		/// </summary>
		void Add(const T& value)
		{
			this->push_back(value);
		}

		/// <summary>
		/// Moves the given value into the end of the vector
		/// </summary>
		void Add(T&& value) noexcept
		{
			this->push_back(std::move(value));
		}

		/// <summary>
		/// Inserts a copy of the given value at the given index
		/// </summary>
		void Insert(ptrdiff_t index, const T& value)
		{
			this->insert(std::vector<T>::begin() + index, value);
		}

		/// <summary>
		/// Moves the given value to the given position in the vector
		/// </summary>
		void Insert(ptrdiff_t index, T&& value) noexcept
		{
			this->insert(std::vector<T>::begin() + index, std::move(value));
		}

		/// <summary>
		/// Moves the given source range into the end of the vector
		/// </summary>
		void AddRange(IDynamicArray<T>&& src, ptrdiff_t srcStart = 0, ptrdiff_t count = -1) noexcept
		{
			InsertRange(this->size(), std::move(src), srcStart, count);
		}

		/// <summary>
		/// Moves the given source range into the vector at the given index
		/// </summary>
		void InsertRange(ptrdiff_t start, IDynamicArray<T>&& src, ptrdiff_t srcStart = 0, ptrdiff_t count = -1) noexcept
		{
			if (count == -1)
				count = (ptrdiff_t)src.GetLength() - count;

			if (count <= 0)
				return;
				
			this->insert((std::vector<T>::begin() + start), 
				std::move_iterator(src.begin() + srcStart), 
				std::move_iterator(src.begin() + srcStart + count)
			);
		}

		template <class InputIterator>
		void InsertRange(ptrdiff_t start, InputIterator first, InputIterator last)
		{
			this->insert((std::vector<T>::begin() + start), first, last);
		}

		/// <summary>
		/// Appends a copy of the given source range to the vector
		/// </summary>
		void AddRange(const IDynamicArray<T>& src, ptrdiff_t srcStart = 0, ptrdiff_t count = -1)
		{
			InsertRange(this->size(), src, srcStart, count);
		}

		/// <summary>
		/// Inserts a copy of the given source range starting at the given index in the vector
		/// </summary>
		void InsertRange(ptrdiff_t start, const IDynamicArray<T>& src, ptrdiff_t srcStart = 0, ptrdiff_t count = -1)
		{
			if (count == -1)
				count = (ptrdiff_t)src.GetLength() - srcStart;

			if (count == 0)
				return;
				
			this->insert((std::vector<T>::begin() + start),
				(src.begin() + srcStart),
				(src.begin() + srcStart + count)
			);
		}

		/// <summary>
		/// Removes the member at the given index
		/// </summary>
		void RemoveAt(size_t index)
		{
#if _CONTAINER_DEBUG_LEVEL > 0
			if ( index >= (this->size()) )
			{
				char buffer[100];
				sprintf_s( buffer, 100, "Array index out of range. Index: %tu, Length %tu", index, (this->size()) );

				throw std::exception(buffer);
			}
#endif

			this->erase(std::vector<T>::begin() + index);
		}

		/// <summary>
		/// Removes the specified contiguous range from the vector
		/// </summary>
		void RemoveRange(size_t index, size_t count)
		{
			if (count == 0)
				return;

#if _CONTAINER_DEBUG_LEVEL > 0
			if ( index >= (this->size()) )
			{
				char buffer[100];
				sprintf_s( buffer, 100, "Array index out of range. Index: %tu, Length %tu", index, (this->size()) );

				throw std::exception(buffer);
			}

			if ( (index + count) >= (this->size() + 1) ) 
			{
				char buffer[100];
				sprintf_s( buffer, 100, "Subrange count out of range. Index: %tu, Length %tu", (index + count), (this->size()) );

				throw std::exception(buffer);
			}
#endif

			this->erase(std::vector<T>::begin() + index, std::vector<T>::begin() + (index + count));
		}

		/// <summary>
		/// Returns the length of the vector.
		/// </summary>
		size_t GetLength() const override { return this->size(); }

		/// <summary>
		/// Returns a copy of the pointer to the backing the vector.
		/// </summary>
		T* GetPtr() override { return this->data(); }

		/// <summary>
		/// Returns a const copy of the pointer to the backing the vector.
		/// </summary>
		const T* GetPtr() const override { return this->data(); }

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		Iterator begin() override { return Iterator(this->data()); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		Iterator end() override { return Iterator(this->data() + this->size()); }

		/// <summary>
		/// Returns iterator pointing to the start of the collection
		/// </summary>
		const Iterator begin() const override { return Iterator(this->data()); }

		/// <summary>
		/// Returns iterator pointing to the end of the collection
		/// </summary>
		const Iterator end() const override { return Iterator(this->data() + this->size()); }

		/// <summary>
		/// Copy assignment operator.
		/// </summary>
		Vector& operator=(const Vector& rhs) noexcept
		{
			std::vector<T>::operator=(rhs);
			return *this;
		}

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		Vector& operator=(Vector&& rhs) noexcept
		{
			std::vector<T>::operator=(std::move(rhs));
			return *this;
		}

		/// <summary>
		/// Provides indexed access to vector member references.
		/// </summary>
		T& operator[](size_t index) override 
		{ 
#if _CONTAINER_DEBUG_LEVEL > 0
			if (index >= this->size())
			{
				char buffer[100];
				sprintf_s(buffer, 100, "Vector index out of range. Index: %tu, Length %tu", index, this->size());

				throw std::exception(buffer);
			}
#endif

			return this->at(index); 
		}

		/// <summary>
		/// Provides indexed access to vector members using constant references.
		/// </summary>
		const T& operator[](size_t index) const override 
		{ 
#if _CONTAINER_DEBUG_LEVEL > 0
			if (index >= this->size())
			{
				char buffer[100];
				sprintf_s(buffer, 100, "Vector index out of range. Index: %tu, Length %tu", index, this->size());

				throw std::exception(buffer);
			}
#endif

		return this->at(index); 
		}
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
		UniqueVector() : Vector<T>() { }

		UniqueVector(const IDynamicArray<T>& other) : Vector<T>(other)
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
		explicit UniqueVector(size_t capacity)
		{
			this->reserve(capacity);
		}

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
		{
			Vector<T>::operator=(std::move(rhs));
			return *this;
		}

		/// <summary>
		/// Returns a new copy of the unique vector.
		/// </summary>
		UniqueVector GetCopy() const
		{
			return UniqueVector(*this);
		}
	};

}
#endif