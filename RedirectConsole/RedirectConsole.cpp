#include <iostream>
#include <cstdint>
#include <string>
#include <Windows.h>
#include <WtsApi32.h>

#include "Logger.h"
#include "WTSSession.h"

static inline void showVersion() {
	LOGGER_DEBUG("RedirectConsole v0.1");
}

auto main() -> int {
	showVersion();
	WTSService::WTSSession sm;
	auto current = sm.getCurrentSession();
	if (current == nullptr) {
		LOGGER_ERROR("Failed to get current session info");
		return -1;
	}
	else if (current->sessionName == "console") {
		LOGGER_ERROR("Please run this program after connecting to RDP");
		return -1;
	}
	else if (current->sessionName.find("rdp-tcp") != 0) {
		LOGGER_ERROR("Unknown current session: " + current->sessionName);
		return -1;
	}
	auto &sessions = sm.getSessions();
	const WTSService::SessionInfo *info = nullptr;
	for (auto &session : sessions){
		if (session.sessionName == "console")
			info = &session;
	}
	if (info == nullptr) {
		LOGGER_ERROR("Failed to get console session");
		return -1;
	}
	sm.disconnectCurrent();
	sm.connect(*info);
	return 0;
}