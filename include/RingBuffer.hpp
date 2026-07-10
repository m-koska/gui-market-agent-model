#pragma once

#include <array>
#include <atomic>
#include <optional>

namespace Utils {

	template <typename T, size_t capacity>
	class RingBuffer {

	private:

		std::array<T, capacity> buffer;

		alignas(64) std::atomic<size_t> head{0};
		alignas(64) std::atomic<size_t> tail{0};

	public:

		bool push(const T& data) {

			const auto current_head = head.load(std::memory_order_relaxed);
			const auto current_tail = tail.load(std::memory_order_acquire);


			const auto new_head = (current_head + 1) % capacity;

			if (new_head == current_tail) {
				return false;
			}

			buffer[current_head] = data;
			head.store(new_head, std::memory_order_release);

			return true;

		}

		std::optional<T> pop() {
			const auto current_head = head.load(std::memory_order_acquire);
			const auto current_tail = tail.load(std::memory_order_relaxed);

			const auto new_tail = (current_tail + 1) % capacity;

			if (current_tail == current_head) {
				return std::nullopt;
			}

			T data = buffer[current_tail];
			tail.store(new_tail, std::memory_order_release);

			return data;
		}
	};

}
