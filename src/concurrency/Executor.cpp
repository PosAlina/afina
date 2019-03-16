#include <afina/concurrency/Executor.h>

namespace Afina {
namespace Concurrency {

void Executor::Stop(bool await) {
	std::unique_lock<std::mutex> lock(mutex);
	state = State::kStopping;
	while (tasks.size() > 0) empty_condition.notify_one();
    if (await) 
		while (state == State::kStopping) stop_condition.wait(lock);
}

void perform(Executor *executor) {
	while (executor->state == Executor::State::kRun) {
		std::unique_lock<std::mutex> lock(executor->mutex);
		auto run_time = std::chrono::system_clock::now() + std::chrono::milliseconds(executor->idle_time);
		while ((executor->tate == Executor::State::kRun) && executor->tasks.empty()) { // Wait
			executor->free_threads++;
			if ((executor->empty_condition.wait_until(lock, run_time) == std::cv_status::timeout) &&
				(executor->threads.size() > executor->low_watermark)) {
					std::thread::id id = std::this_thread::get_id();
					auto iterator = std::find_if(executor->threads.begin(), executor->threads.end(), [=](std::thread &t) { return (t.get_id() == id); });
					if (iterator != executor->threads.end()) {
						executor->free_threads--;
						executor->threads.erase(iterator);
					}
					return;
			}
			else executor->empty_condition.wait(lock);
			executor->free_threads--; 
		} // Stop wait
		if (executor->tasks.empty()) continue;
		task = executor->tasks.front();
		executor->tasks.pop_front();
		task();
		if (executor->state == State::kStopping) {
            std::thread::id id = std::this_thread::get_id();
			auto iterator = std::find_if(executor->threads.begin(), executor->threads.end(), [=](std::thread &t) { return (t.get_id() == id); });
			if (iterator != executor->threads.end()) {
				executor->free_threads--;
				executor->threads.erase(iterator);
			}
			if (!executor->threads.size()) {
				executor->state = State::kStopped;
				executor->stop_condition.notify_one();
			}
        }
	}
}

}
} // namespace Afina
