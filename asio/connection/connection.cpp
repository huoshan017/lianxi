#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <thread>
#include <functional>
#include <chrono>
#include <iostream>

using namespace  boost::asio;

struct connection : public std::enable_shared_from_this<connection> {
	typedef boost::system::error_code error_code;
	typedef std::shared_ptr<connection> ptr;
	connection(io_service& service) : sock_(service), started_(true) {}
	void start(ip::tcp::endpoint ep) {
		sock_.async_connect(ep, std::bind(&connection::on_connect, shared_from_this(), std::placeholders::_1));
	}
	void stop() {
		if (!started_) {
			return;
		}
		started_ = false;
		sock_.close();
	}
	bool started() {
		return started_;
	}

private:
	void on_connect(const error_code& err) {
		if (!err) do_read();
		else stop();
	}
	void on_read(const error_code& err, size_t bytes) {
		(void)err;
		if (!started()) return;
		std::string msg(read_buffer_, bytes);
		if (msg == "can_login") do_write("access_data");
		else if (msg.find("data") == 0) process_data(msg);
		else if (msg == "login_fail") stop();
	}
	void on_write(const error_code& err, size_t bytes) {
		(void)err;
		(void)bytes;
		do_read();
	}
	void do_read() {
		sock_.async_read_some(buffer(read_buffer_), std::bind(&connection::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void do_write(const std::string& msg) {
		if (!started()) return;
		std::copy(msg.begin(), msg.end(), write_buffer_);
		sock_.async_write_some(buffer(write_buffer_, msg.size()), std::bind(&connection::on_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void process_data(const std::string& msg) {
		(void)msg;
	}
private:
	ip::tcp::socket sock_;
	enum {max_msg = 1024};
	char read_buffer_[max_msg];
	char write_buffer_[max_msg];
	bool started_;
};

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
	io_service service;
	connection::ptr(new connection(service))->start(ep);
}
