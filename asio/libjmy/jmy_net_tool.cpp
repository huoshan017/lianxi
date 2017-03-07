#include "jmy_net_tool.h"

JmyNetTool::JmyNetTool()
	: up_stream_(0),
	down_stream_(0),
	up_last_stream_(0),
	down_last_stream_(0),
	last_stream_(0),
	last_up_bps_(0),
	last_down_bps_(0),
	last_bps_(0),
	interval_(1),
	state_(STATE_STOP)
{
}

JmyNetTool::~JmyNetTool()
{
}

void JmyNetTool::reset()
{
	up_stream_ = 0;
	down_stream_ = 0;
	state_ = STATE_STOP;
}

bool JmyNetTool::start()
{
	if (state_ != STATE_STOP)
		return false;
	state_ = STATE_START;
	return true;
}

bool JmyNetTool::pause()
{
	if (state_ != STATE_START)
		return false;
	state_ = STATE_PAUSE;
	pause_tick_ = std::chrono::system_clock::now();
	return true;
}

bool JmyNetTool::resume()
{
	if (state_ != STATE_PAUSE)
		return false;
	state_ = STATE_START;
	return true;
}

bool JmyNetTool::stop()
{
	if (state_ != STATE_START)
		return false;
	state_ = STATE_STOP;
	return true;
}

void JmyNetTool::addUpStream(uint32_t count)
{
	if (state_ != STATE_START)
		return;
	up_stream_ += count;
}

void JmyNetTool::addDownStream(uint32_t count)
{
	if (state_ != STATE_START)
		return;
	down_stream_ += count;
}

uint32_t JmyNetTool::getSendBps()
{
	auto now = std::chrono::system_clock::now();
	auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_up_tick_).count();
	uint32_t bps = 0;
	if (d >= interval_ * 1000) {
		bps = (up_stream_ - up_last_stream_) * 1000 / d;
		up_last_stream_ = up_stream_;
		last_up_bps_ = bps;
		last_up_tick_ = now;
	} else {
		bps = last_up_bps_;
	}
	return bps;
}

uint32_t JmyNetTool::getRecvBps()
{
	auto now = std::chrono::system_clock::now();
	auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_down_tick_).count();
	uint32_t bps = 0;
	if (d >= interval_ * 1000) {
		bps = (down_stream_ - down_last_stream_) * 1000 / d;
		down_last_stream_ = down_stream_;
		last_down_bps_ = bps;
		last_down_tick_ = now;
	} else {
		bps = last_down_bps_;
	}
	return bps;
}

uint32_t JmyNetTool::getBps()
{
	auto now = std::chrono::system_clock::now();
	auto d = std::chrono::duration_cast<std::chrono::milliseconds>(now-last_tick_).count();
	uint32_t bps = 0;
	if (d >= interval_ * 1000) {
		bps = (up_stream_ + down_stream_ - last_stream_) * 1000 / d;
		last_stream_ = (up_stream_ + down_stream_);
		last_bps_ = bps;
		last_tick_ = now;
	} else {
		bps = last_bps_;
	}
	return bps;
}
