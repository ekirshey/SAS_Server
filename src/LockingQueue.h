#pragma once

namespace Containers {

	template<typename T>
	struct LockingQueue {
		LockingQueue() {}

		void enqueue(T& packet) {
			std::unique_lock<std::mutex> lk(_mx);
			_queue.push_back(std::move(packet));
		}

		size_t dequeue(T& obj) {
			size_t size;
			{
				std::unique_lock<std::mutex> lk(_mx);
				_peak = std::max(_peak, _queue.size());
				if (_queue.size() <= 0)
					return 0;
				size = _queue.size();
				obj = std::move(_queue.front());
				_queue.pop_front();
			}
			return size;
		}

		size_t peak_depth() const {
			std::lock_guard<std::mutex> lk(_mx);
			return _peak;
		}

	private:
		mutable std::mutex _mx;

		size_t _peak = 0;
		std::deque<T> _queue;
	};
}
