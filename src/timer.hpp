# include <chrono>
# include <optional>

typedef std::chrono::high_resolution_clock Clock;

struct Timer {
	std::optional<Clock::time_point> start_time;
	Clock::duration elapsed_time;
	
	Timer() : start_time(std::nullopt), elapsed_time(Clock::duration::zero()) { }
	void start(){
		start_time = Clock::now();
	}
	void stop(){
		elapsed_time += Clock::now() - start_time.value();
		start_time = std::nullopt;
	}
	void reset(){
		start_time = std::nullopt;
		elapsed_time = Clock::duration::zero();
	}
	int ms_elapsed(){
		auto duration = elapsed_time;
		if (start_time.has_value()) duration += Clock::now() - start_time.value();
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	}
};
