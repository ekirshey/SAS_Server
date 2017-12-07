#pragma once
#include <mutex>
#include <deque>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>

/*
// For Reference
using namespace std::chrono_literals;
using Clock = std::chrono::high_resolution_clock;
*/

namespace Containers {
	template<typename T>
	struct BlockingQueue {
		BlockingQueue(size_t max = 50) : _max(max) {}

		void enqueue(T& packet) {
			std::unique_lock<std::mutex> lk(_mx);
			_cv.wait(lk, [this] { return _queue.size() < _max; });
			_queue.push_back(std::move(packet));

			_cv.notify_one();
		}

		T dequeue() {
			T packet;
			{
				std::unique_lock<std::mutex> lk(_mx);
				_peak = std::max(_peak, _queue.size());
				_cv.wait(lk, [this] { return _queue.size() > 0; });
				packet = std::move(_queue.front());
				_queue.pop_front();
				lk.unlock();
				_cv.notify_one();
			}

			return packet;
		}

		size_t peak_depth() const {
			std::lock_guard<std::mutex> lk(_mx);
			return _peak;
		}

	private:
		mutable std::mutex _mx;
		mutable std::condition_variable _cv;

		size_t _max = 50;
		size_t _peak = 0;
		std::deque<T> _queue;
	};


}
