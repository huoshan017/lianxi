#pragma once

enum UserState {
	USER_STATE_IDLE = 0,
	USER_STATE_LOGINING = 1,
	USER_STATE_VERIFIED = 2,
	USER_STATE_ENTERING_GAME = 3,
	USER_STATE_IN_GAME = 4,
	USER_STATE_QUIT_GAME = 5,
};