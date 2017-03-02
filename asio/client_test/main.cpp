#include <boost/asio.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstring>
#include <memory>

using namespace boost::asio;

const char* str[] = {
	"111111111",
	"222222222",
	"333333333",
	"444444444"
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
			std::cout << "send " << bytes_transferred << " bytes" << std::endl;
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
			[&sock, &receiving, &nrecv](const boost::system::error_code& err, size_t bytes_transferred){
		if (!err) {
			nrecv += bytes_transferred;
			std::cout << "recv " << bytes_transferred << " bytes" << std::endl;
		} else {
			sock.close();
			std::cout << "recv bytes failed" << std::endl;
		}
		receiving = false;	
	});
	receiving = true;
	return 1;
}

void output_recv(const char* buf, int len){
	static int offset = 0;
	static char tmp[1024];
	while (true) {
		if (len < 2+offset) {
			std::cout << "not enough length get head" << std::endl;
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
		std::cout << "msg_id: " << msg_id << ", body: " << tmp << std::endl;
		offset += (data_len + 2);
	}
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	io_service service;

	ip::tcp::socket sock(service);
	sock.connect(ip::tcp::endpoint(ip::address::from_string("192.168.0.200"), 10000));

	int i = 0;
	int s = sizeof(str)/sizeof(str[0]);
	bool send = false;
	char buf[2048];
	bool receiving = false;
	int nrecv = 0;
	while(true) {
		if (i < s) {
			send_func(sock, str[i], send, i);
		}
		size_t ss = service.poll();
		if (ss == 0) {
			break;
		}
		if (recv_func(sock, buf+nrecv, sizeof(buf)-1, receiving, nrecv) > 0) {
		
		}
		output_recv(buf, nrecv);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	return 0;
}
