// SafeQueue.h

#pragma once 
#include <mutex> 
#include <queue> 
// Thread safe implementation of a Queue using a std::queue

template <typename T>
class SafeQueue {
private:
	std::queue<T> m_queue; //����ģ�庯���������
	std::mutex m_mutex;//���ʻ����ź���

public:
	//�չ��캯��
	SafeQueue() {
	}

	//�������캯��
	SafeQueue(SafeQueue& other) {
		//TODO:
	}

	//��������
	~SafeQueue() {
	}

	//�����Ƿ�Ϊ��
	bool empty() {
		std::unique_lock<std::mutex> lock(m_mutex); //�����źű�����������ֹm_queue���ı�
		return m_queue.empty();
	}

	int size() {
		std::unique_lock<std::mutex> lock(m_mutex); //�����źű�����������ֹm_queue���ı�
		return m_queue.size();
	}

	//�������Ԫ��
	void enqueue(T& t) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.push(t);
	}

	//����ȡ��Ԫ��
	bool dequeue(T& t) {
		std::unique_lock<std::mutex> lock(m_mutex); //���м���
		if (m_queue.empty()) {
			return false;
		}
		t = std::move(m_queue.front()); //ȡ������Ԫ�أ����ض���Ԫ��ֵ����������ֵ����
		m_queue.pop(); //������ӵĵ�һ��Ԫ��
		return true;
	}
};