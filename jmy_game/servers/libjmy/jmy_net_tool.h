#pragma once
#include <stdint.h>
#include <chrono>

class JmyNetTool
{
public:
	JmyNetTool();
	~JmyNetTool();

	void reset();

	bool start();
	bool pause();
	bool resume();
	bool stop();

	void addUpStream(uint32_t count);
	void addDownStream(uint32_t count);

	uint32_t getSendBps();
	uint32_t getRecvBps();
	uint32_t getBps();

private:
	uint64_t up_stream_;
	uint64_t down_stream_;
	uint32_t up_last_stream_;
	uint32_t down_last_stream_;
	uint32_t last_stream_;
	uint32_t last_up_bps_;
	uint32_t last_down_bps_;
	uint32_t last_bps_;
	std::chrono::system_clock::time_point start_;
	std::chrono::system_clock::time_point pause_tick_;
	std::chrono::system_clock::time_point last_up_tick_;
	std::chrono::system_clock::time_point last_down_tick_;
	std::chrono::system_clock::time_point last_tick_;
	uint32_t interval_; // time span to get Bps in seconds
	enum State {
		STATE_STOP = 1,
		STATE_START = 2,
		STATE_PAUSE = 3,
	} state_;
};
