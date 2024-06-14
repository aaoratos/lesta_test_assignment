#pragma once

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <iterator>

namespace containers {

namespace detail {

template<class size_type, size_type N>
struct index_wrapper
{
	inline static constexpr size_type increment(size_type idx) noexcept
	{
		return (idx + 1) % N;
	}

	inline static constexpr size_type decrement(size_type idx) noexcept
	{
		return (idx + N - 1) % N;
	}
};

template<class T, bool = std::is_trivially_destructible<T>::value>
union optional_storage
{
	struct empty_t {};

	empty_t empty_;
	T value_;

	inline explicit constexpr optional_storage() noexcept
		: empty_() {}

	inline explicit constexpr optional_storage(const T &value) noexcept
		: value_(value) {}

	inline explicit constexpr optional_storage(T &&value)
		: value_(std::move(value)) {}

	~optional_storage() {}
};

template<class T>
union optional_storage<T, true>
{
	struct empty_t {};

	empty_t empty_;
	T value_;

	inline explicit constexpr optional_storage() noexcept
		: empty_() {}

	inline explicit constexpr optional_storage(const T &value) noexcept
		: value_(value) {}

	inline explicit constexpr optional_storage(T &&value)
		: value_(std::move(value)) {}

	~optional_storage() = default;
};

template<class S, class TC, std::size_t N>
class iterator
{
	template<class, class, std::size_t>
	friend class iterator;

	S *buf_;
	std::size_t pos_;
	std::size_t left_in_forward_;

	using wrapper_t = detail::index_wrapper<std::size_t, N>;

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = TC;
	using difference_type = std::ptrdiff_t;
	using pointer = value_type *;
	using reference = value_type &;

	explicit constexpr iterator() noexcept
		: buf_(nullptr), pos_(0), left_in_forward_(0) {}

	explicit constexpr iterator(S *buf,
	                            std::size_t pos,
	                            std::size_t left_in_forward) noexcept
			: buf_(buf), pos_(pos), left_in_forward_(left_in_forward) {}

	template<class TSnc, class Tnc>
	constexpr explicit iterator(const iterator<TSnc, Tnc, N> &other) noexcept
			: buf_(other.buf_),
			pos_(other.pos_),
			left_in_forward_(other.left_in_forward_) {}

	template<class TSnc, class Tnc>
	constexpr iterator &operator=(const iterator<TSnc, Tnc, N> &other) noexcept
	{
		if (other == *this) {
			return *this;
		}

		buf_ = other.buf_;
		pos_ = other.pos_;
		left_in_forward_ = other.left_in_forward_;

		return *this;
	}

	constexpr reference operator*() const noexcept
	{
		return (buf_ + pos_)->value_;
	}

	constexpr pointer operator->() const noexcept
	{
		return std::addressof((buf_ + pos_)->value_);
	}

	constexpr iterator &operator++() noexcept
	{
		pos_ = wrapper_t::increment(pos_);
		--left_in_forward_;
		return *this;
	}

	constexpr iterator &operator--() noexcept
	{
		pos_ = wrapper_t::decrement(pos_);
		++left_in_forward_;
		return *this;
	}

	constexpr iterator operator++(int) noexcept
	{
		iterator tmp = *this;
		pos_ = wrapper_t::increment(pos_);
		--left_in_forward_;
		return tmp;
	}

	constexpr iterator operator--(int) noexcept
	{
		iterator tmp = *this;
		pos_ = wrapper_t::decrement(pos_);
		++left_in_forward_;
		return tmp;
	}

	template<class Tx, class Ty>
	constexpr bool operator==(const iterator<Tx, Ty, N> &rhs) const noexcept
	{
		return rhs.left_in_forward_ == left_in_forward_
		       && pos_ == rhs.pos_
		       && rhs.buf_ == buf_;
	}

	template<class Tx, class Ty>
	constexpr bool operator!=(const iterator<Tx, Ty, N> &rhs) const noexcept
	{
		return !(operator==(rhs));
	}
};

} // namespace detail

template<typename T, std::size_t N>
class circular_buffer
{
public:
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T &;
	using const_reference = const T &;
	using pointer = T *;
	using const_pointer = const T *;
	using iterator = detail::iterator<detail::optional_storage<T>, T, N>;
	using const_iterator = detail::iterator<const detail::optional_storage<T>, const T, N>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
	using wrapper_t = detail::index_wrapper<size_type, N>;
	using storage_type = detail::optional_storage<T>;

	size_type head_;
	size_type tail_;
	size_type size_;
	storage_type buffer_[N];

	inline void destroy(size_type idx) noexcept
	{
		buffer_[idx].value_.~T();
	}

	inline void copy_buffer(const circular_buffer &other)
	{
		const_iterator first = other.cbegin();
		const const_iterator last = other.cend();

		for (; first != last; ++first) {
			push_back(*first);
		}
	}

	inline void move_buffer(circular_buffer &&other)
	{
		const_iterator first = other.cbegin();
		const const_iterator last = other.cend();

		for (; first != last; ++first) {
			emplace_back(*first);
		}
	}

public:
	constexpr explicit circular_buffer()
			: head_(1), tail_(0), size_(0), buffer_() {}

	explicit circular_buffer(size_type count, const T& value = T())
		: head_(0), tail_(count - 1), size_(count), buffer_()
	{
		if (size_ > N) {
			throw std::out_of_range("circular_buffer<T, N>(size_type count,"
									"const T&) count exceeded N");
		}

		if (size_ != 0) {
			for (size_type i = 0; i < count; ++i) {
				new(std::addressof(buffer_[i].value_)) T(value);
			}
		} else {
			head_ = 1;
		}
	}

	template <typename InputIt>
	circular_buffer(InputIt first, InputIt last)
		: head_(0), tail_(0), size_(0), buffer_()
	{
		if (first == last) {
			head_ = 1;
		} else {
			for (; first != last; ++first, ++size_) {
				if (size_ >= N) {
					throw std::out_of_range("circular_buffer<T, N>(InputIt "
											"first, InputIt last) "
											"distance exceeded N");
				}
				new(std::addressof(buffer_[size_].value_)) T(*first);
			}
			tail_ = size_ - 1;
		}
	}

	circular_buffer(std::initializer_list<T> init)
		: head_(0), tail_(init.size() - 1), size_(init.size()), buffer_()
	{
		if (size_ > N) {
			throw std::out_of_range("circular_buffer<T, N>(std::initializer_list"
									"<T> init) init.size() > N");
		}

		if (size_ == 0) {
			head_ = 1;
		}

		storage_type* buf_ptr = buffer_;
		for (auto it = init.begin(), end = init.end(); it != end; ++it, ++buf_ptr) {
			new(std::addressof(buf_ptr->value_)) T(*it);
		}

	}

	circular_buffer(const circular_buffer& other)
		: head_(1), tail_(0), size_(0), buffer_()
	{
		copy_buffer(other);
	}

	circular_buffer& operator=(const circular_buffer& other)
	{
		if (other == *this) {
			return *this;
		}

		clear();
		copy_buffer(other);
		return *this;
	}

	circular_buffer(circular_buffer&& other) noexcept
		: head_(1), tail_(0), size_(0), buffer_()
	{
		move_buffer(std::move(other));
	}

	circular_buffer& operator=(circular_buffer&& other)
	{
		clear();
		move_buffer(std::move(other));
		return *this;
	}

	~circular_buffer() { clear(); }

public:
	[[nodiscard]] constexpr bool is_empty() const noexcept
	{
		return size_ == 0;
	}

	[[nodiscard]] constexpr bool is_full() const noexcept
	{
		return size_ == N;
	}

	[[nodiscard]] constexpr size_type size() const noexcept
	{
		return size_;
	}

	[[nodiscard]] constexpr size_type capacity() const noexcept
	{
		return N;
	}

	constexpr reference front() noexcept
	{
		return buffer_[head_].value_;
	}

	constexpr const_reference front() const noexcept
	{
		return buffer_[head_].value_;
	}

	constexpr reference back() noexcept
	{
		return buffer_[tail_].value_;
	}

	constexpr const_reference back() const noexcept
	{
		return buffer_[tail_].value_;
	}

	constexpr const_pointer data() const noexcept
	{
		return std::addressof(buffer_[0].value_);
	}

	void push_back(const value_type& value)
	{
		size_type new_tail;
		if (size_ == N) {
			new_tail = head_;
			head_ = wrapper_t::increment(head_);
			--size_;
			buffer_[new_tail].value_ = value;
		} else {
			new_tail = wrapper_t::increment(tail_);
			new(std::addressof(buffer_[new_tail].value_)) T(value);
		}

		tail_ = new_tail;
		++size_;
	}

	void push_front(const value_type& value)
	{
		size_type new_head = 0;
		if (size_ == N) {
			new_head = tail_;
			tail_ = wrapper_t::decrement(tail_);
			--size_;
			buffer_[new_head].value_ = value;
		} else {
			new_head = wrapper_t::decrement(head_);
			new(std::addressof(buffer_[new_head].value_)) T(value);
		}

		head_ = new_head;
		++size_;
	}

	void push_back(value_type&& value)
	{
		size_type new_tail;

		if (size_ == N) {
			new_tail = head_;
			head_ = wrapper_t::increment(head_);
			--size_;
			buffer_[new_tail].value_ = std::move(value);
		} else {
			new_tail = wrapper_t::increment(tail_);
			new(std::addressof(buffer_[new_tail].value_)) T(std::move(value));
		}

		tail_ = new_tail;
		++size_;
	}

	void push_front(value_type&& value)
	{
		size_type new_head = 0;

		if (size_ == N) {
			new_head = tail_;
			tail_    = wrapper_t::decrement(tail_);
			--size_;
			buffer_[new_head]._value = std::move(value);
		} else {
			new_head = wrapper_t::decrement(head_);
			new(JM_CB_ADDRESSOF(buffer_[new_head]._value))
					T(std::move_if_noexcept(value));
		}

		head_ = new_head;
		++size_;
	}

	template <typename... Args>
	void emplace_back(Args&&... args)
	{
		size_type new_tail;
		if (size_ == N) {
			new_tail = head_;
			head_ = wrapper_t::increment(head_);
			--size_;
			destroy(new_tail);
		} else {
			new_tail = wrapper_t::increment(tail_);
		}

		new(std::addressof(buffer_[new_tail].value_))
			value_type(std::forward<Args>(args)...);
		tail_ = new_tail;
		++size_;
	}

	template <typename... Args>
	void emplace_front(Args&&... args)
	{
		size_type new_head;
		if (size_ == N) {
			new_head = tail_;
			tail_ = wrapper_t::increment(tail_);
			--size_;
			destroy(new_head);
		} else {
			new_head = wrapper_t::increment(head_);
		}

		new(std::addressof(buffer_[new_head].value_))
				value_type(std::forward<Args>(args)...);
		head_ = new_head;
		++size_;
	}

	constexpr void pop_back() noexcept
	{
		size_type old_tail = tail_;
		--size_;
		tail_ = wrapper_t::decrement(tail_);
		destroy(old_tail);
	}

	constexpr void pop_front() noexcept
	{
		size_type old_head = head_;
		--size_;
		head_ = wrapper_t::increment(head_);
		destroy(old_head);
	}

	constexpr void clear() noexcept
	{
		while (size_ != 0) {
			pop_back();
		}

		head_ = 1;
		tail_ = 0;
	}

public:
	constexpr iterator begin() noexcept
	{
		if (size_ == 0) {
			return end();
		}

		return iterator(buffer_, head_, size_);
	}

	constexpr const_iterator begin() const noexcept
	{
		if (size_ == 0) {
			return end();
		}

		return const_iterator(buffer_, head_, size_);
	}

	constexpr const_iterator cbegin() const noexcept
	{
		if (size_ == 0) {
			return cend();
		}

		return const_iterator(buffer_, head_, size_);
	};

	constexpr reverse_iterator rbegin() noexcept
	{
		if (size_ == 0) {
			return rend();
		}

		return reverse_iterator(iterator(buffer_, head_, size_));
	}

	constexpr const_reverse_iterator rbegin() const noexcept
	{
		if (size_ == 0) {
			return rend();
		}

		return const_reverse_iterator(const_iterator(buffer_, head_, size_));
	}

	constexpr const_reverse_iterator crbegin() const noexcept
	{
		if (size_ == 0) {
			return crend();
		}

		return const_reverse_iterator(const_iterator(buffer_, head_, size_));
	}

	constexpr iterator end() noexcept
	{
		return iterator(buffer_, wrapper_t::increment(tail_), 0);
	}

	constexpr const_iterator end() const noexcept
	{
		return const_iterator(buffer_, wrapper_t::increment(tail_), 0);
	}

	constexpr const_iterator cend() const noexcept
	{
		return const_iterator(buffer_, wrapper_t::increment(tail_), 0);
	}

	constexpr reverse_iterator rend() noexcept
	{
		return reverse_iterator(iterator(buffer_, wrapper_t::increment(tail_), 0));
	}

	constexpr const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(iterator(buffer_, wrapper_t::increment(tail_), 0));
	}

	constexpr const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(iterator(buffer_, wrapper_t::increment(tail_), 0));
	}
};

} // namespace containers