namespace containers {

enum class memory_model { array, list };

template <typename T, memory_model Base = memory_model::array>
class circular_buffer;

template <typename T>
class circular_buffer<T, memory_model::list>
{
private:
	struct Node
	{
		T data;
		Node* next;
		Node* prev;

		explicit Node(const T& value)
			: data(value), next(nullptr), prev(nullptr) {}
	};

public:
	explicit circular_buffer(std::size_t size)
		: _capacity(size), _head(nullptr), _tail(nullptr), _size(0) {}

	~circular_buffer() { clear(); }

	[[nodiscard]] bool is_empty() const
	{
		return _size == 0;
	}

	[[nodiscard]] bool is_full() const
	{
		return _size == _capacity;
	}

	[[nodiscard]] std::size_t size() const
	{
		return _size;
	}

	void push(const T& item)
	{
		if (is_full()) {
			pop();
		}

		Node* new_node = new Node(item);

		if (is_empty()) {
			_head = _tail = new_node;
		} else {
			_tail->next = new_node;
			new_node->prev = _tail;
			_tail = new_node;
		}

		++_size;
	}

	T pop()
	{
		T item = _head->data;
		Node* tmp = _head;
		_head = _head->next;

		if (_head) {
			_head->prev = nullptr;
		}

		delete tmp;
		--_size;

		return item;
	}

private:
	std::size_t _capacity;
	Node* _head;
	Node* _tail;
	std::size_t _size;

	void clear()
	{
		while (!is_empty()) {
			pop();
		}
	}
};

template <typename T>
class circular_buffer<T, memory_model::array>
{
public:
	explicit circular_buffer(std::size_t size)
			: _capacity(size),
			_buffer(new T[size]),
			_head(0),
			_tail(0),
			_size(0) {}

	~circular_buffer()
	{
		delete[] _buffer;
	}

	void push(const T& item)
	{
		_buffer[_head] = item;
		_head = (_head + 1) % _capacity;
		if (_head == _tail) {
			_tail = (_tail + 1) % _capacity;
		}
		if (_size < _capacity) {
			++_size;
		}
	}

	T pop()
	{
		T item = _buffer[_tail];
		_tail = (_tail + 1) % _capacity;
		--_size;
		return item;
	}

	[[nodiscard]] bool is_empty() const
	{
		return _size == 0;
	}

	[[nodiscard]] bool is_full() const
	{
		return _size == _capacity;
	}

	[[nodiscard]] std::size_t size() const
	{
		return _size;
	}

private:
	std::size_t _capacity;
	T* _buffer;
	std::size_t _head;
	std::size_t _tail;
	std::size_t _size;
};

} // namespace containers
