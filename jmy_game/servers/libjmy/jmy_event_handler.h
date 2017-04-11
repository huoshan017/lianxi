#pragma once

#include "jmy_datatype.h"
#include <memory>
#include <cassert>
#include <iostream>

class JmyEventHandlerManager
{
public:
	JmyEventHandlerManager() {
		std::memset(handlers_, 0, sizeof(handlers_));
	}
	~JmyEventHandlerManager() {
	}

	void setHandler(int event_id, jmy_event_handler handler) {
		assert(event_id>JMY_EVENT_NONE && event_id<JMY_EVENT_MAX_COUNT);
		handlers_[event_id] = handler;
	}

	void init(JmyId2EventHandler* id2handlers, size_t len) {
		for (size_t i=0; i<len; ++i)
			setHandler(id2handlers[i].event_id, id2handlers[i].handler);
	}

	bool hasConnectHandler() const {
		return handlers_[JMY_EVENT_CONNECT] != nullptr;
	}
	bool hasDisconnectHandler() const {
		return handlers_[JMY_EVENT_DISCONNECT] != nullptr;
	}
	bool hasTickHandler() const {
		return handlers_[JMY_EVENT_TICK] != nullptr;
	}
	bool hasTimerHandler() const {
		return handlers_[JMY_EVENT_TIMER] != nullptr;
	}
	bool hasEventHandler(int event_id) {
		assert(event_id>JMY_EVENT_NONE && event_id<JMY_EVENT_MAX_COUNT);
		return handlers_[event_id] != nullptr;
	}

	void setConnectHandler(jmy_event_handler handler) {
		handlers_[JMY_EVENT_CONNECT] = handler;
	}

	void setDisconnectHandler(jmy_event_handler handler) {
		handlers_[JMY_EVENT_DISCONNECT] = handler;
	}

	void setTickHandler(jmy_event_handler handler) {
		handlers_[JMY_EVENT_TICK] = handler;
	}

	void setTimerHandler(jmy_event_handler handler) {
		handlers_[JMY_EVENT_TIMER] = handler;
	}

	void setBaseHandlers(const JmyBaseEventHandlers& handlers) {
		handlers_[JMY_EVENT_CONNECT] = handlers.conn_handler;
		handlers_[JMY_EVENT_DISCONNECT] = handlers.disconn_handler;
		handlers_[JMY_EVENT_TICK] = handlers.tick_handler;
		handlers_[JMY_EVENT_TIMER] = handlers.timer_handler;
	}

	// event handle
	int onConnect(JmyEventInfo* info) {
		if (handlers_[JMY_EVENT_CONNECT]) {
			return handlers_[JMY_EVENT_CONNECT](info);
		} else {
			return onDefaultConnect(info);
		}
	}
	int onDisconnect(JmyEventInfo* info) {
		if (handlers_[JMY_EVENT_DISCONNECT]) {
			return handlers_[JMY_EVENT_DISCONNECT](info);
		} else {
			return onDefaultDisconnect(info);
		}
	}
	int onTick(JmyEventInfo* info) {
		if (handlers_[JMY_EVENT_TICK]) {
			return handlers_[JMY_EVENT_TICK](info);
		} else {
			return onDefaultTick(info);
		}
	}
	int onTimer(JmyEventInfo* info) {
		if (handlers_[JMY_EVENT_TIMER]) {
			return handlers_[JMY_EVENT_TIMER](info);
		} else {
			return onDefaultTimer(info);
		}
	}
	int onEvent(JmyEventInfo* info) {
		assert(info->event_id>JMY_EVENT_NONE && info->event_id<JMY_EVENT_MAX_COUNT);
		if (handlers_[info->event_id]) {
			return handlers_[info->event_id](info);
		} else {
			return onDefaultEvent(info);
		}
	}

protected:
	int onDefaultConnect(JmyEventInfo* info) {
		(void)info;
		return 0;
	}
	int onDefaultDisconnect(JmyEventInfo* info) {
		(void)info;
		return 0;
	}
	int onDefaultTick(JmyEventInfo* info) {
		(void)info;
		return 0;
	}
	int onDefaultTimer(JmyEventInfo* info) {
		(void)info;
		return 0;
	}
	int onDefaultEvent(JmyEventInfo* info) {
		(void)info;
		return 0;
	}

protected:
	jmy_event_handler handlers_[JMY_EVENT_MAX_COUNT];
};
