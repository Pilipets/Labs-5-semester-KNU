#pragma once
#include <thread>
#include <unordered_map>

#include "Concepts.h"
namespace thread_sync {
	class BlackWhiteBakeryLock : public BasicLockable, public ThreadMapping {
	private:
		int _produce_ticket(int num);
	public:
		BlackWhiteBakeryLock(int n = 40);
		~BlackWhiteBakeryLock();
		void lock() override;
		void unlock() override;
	private:
		enum class Color: bool {
			white, black
		};
		volatile Color _shared_color;
		volatile Color *_my_color;
		volatile bool* _choosing;
		volatile int* _number;
		const int _n;
	};
}