#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>
#include <memory>

using namespace boost::asio;

const char* str[] = {
	"111111111",
	"2222222222",
	"33333333333",
	"444444444444",
	"5555555555555",
	"66666666666666",
	"777777777777777",
	"8888888888888888",
	"99999999999999999",
	"000000000000000000"
};

int send_func(ip::tcp::socket& sock, const char* str, bool& sending, int& i) {
	if (sending)
		return 0;

	char buf[1024];
	int data_len = 2+strlen(str);
	buf[0] = (data_len>>8)&0xff;
	buf[1] = data_len&0xff;
	buf[2] = (1>>8)&0xff;
	buf[3] = 1&0xff;
	std::memcpy(buf+4, str, strlen(str));

	sock.async_send(boost::asio::buffer(buf, 4+strlen(str)),
		[&sock, &sending, &i](const boost::system::error_code& err, size_t bytes_transferred){
		if (!err) {
			i += 1;
			std::cout << "sent " << bytes_transferred << " bytes" << std::endl;
		} else {
			sock.close();
			std::cout << "send bytes failed" << std::endl;
		}
		sending = false;
	});
	sending = true;
	return 1;
}

int recv_func(ip::tcp::socket& sock, char* buf, int len, bool& receiving, int& nrecv)
{
	if (receiving)
		return 0;

	sock.async_read_some(boost::asio::buffer(buf, len), 
			[&sock, buf, len, &receiving, &nrecv](const boost::system::error_code& err, size_t bytes_transferred){
		if (!err) {
			if (bytes_transferred == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			} else {
				nrecv += bytes_transferred;
			}
			std::cout << "recv " << bytes_transferred << " bytes,  nrecv " << nrecv << ",  len " << len << ",  receiving " << receiving << std::endl;
			receiving = false;
			recv_func(sock, buf+bytes_transferred, len-bytes_transferred, receiving, nrecv);
		} else {
			sock.close();
			std::cout << "recv bytes failed" << std::endl;
		}
	});
	receiving = true;
	//std::cout << "receiving..." << std::endl;
	return 1;
}

void output_recv(const char* buf, int len){
	static int offset = 0;
	static char tmp[1024];
	while (true) {
		if (len-offset < 2) {
			//std::cout << "not enough length get head" << std::endl;
			break;
		}
		int data_len = (((int)(buf[offset])<<8)&0xff00) + ((int(buf[offset+1]))&0xff);
		if (len-2 < data_len) {
			std::cout << "not enough length to get data" << std::endl;
			break;
		}
		int msg_id = ((int(buf[offset+2])<<8)&0xff00) + ((int(buf[offset+3]))&0xff);
		std::memcpy(tmp, buf+offset+4, data_len-2);
		tmp[data_len-2] = '\0';
		std::cout << "received   msg_id: " << msg_id << ", body: " << tmp << std::endl;
		offset += (data_len + 2);
	}
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	io_service service;

	ip::tcp::socket sock(service);
	sock.connect(ip::tcp::endpoint(ip::address::from_string("127.0.0.1"), 10000));
	std::cout << "connect success" << std::endl;

	int i = 0;
	int s = sizeof(str)/sizeof(str[0]);
	bool send = false;
	char* buf = new char[2048*10000];
	int nrecv = 0;
	bool receiving = false;
	recv_func(sock, buf, sizeof(buf)-1, receiving, nrecv);
	while (true) {
		if (i < s) {
			send_func(sock, str[i], send, i);
		}
		if (i >= s) i = 0;
		size_t ss = service.poll();
		if (ss == 0) {
		}
		output_recv(buf, nrecv);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	service.stop();
	return 0;
}
