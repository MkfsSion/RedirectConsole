#include <algorithm>
#include <cctype>

#include <Windows.h>
#include <winternl.h>

#include "Logger.h"
#include "WTSSession.h"


namespace WTSService {
	WTSSession::WTSSession() {
		DWORD level = 1;
		DWORD count = 0;
		PWTS_SESSION_INFO_1A sessions;
		auto result = WTSEnumerateSessionsExA(WTS_CURRENT_SERVER_HANDLE, &level, 0, &sessions, &count);
		if (!result || count == 0)
		{
			LOGGER_ERROR("Failed to get sessions information");
			LOGGER_ERROR("Error reason: " + std::to_string(GetLastError()));
		}
		LOGGER_DEBUG("Successfully get " + std::to_string(count) + " sessions");
		for (int i = 0; i < static_cast<int>(count); i++) {
			SessionInfo info;
			info.sessionId = sessions[i].SessionId;
			info.sessionName = sessions[i].pSessionName;
			std::transform(info.sessionName.begin(), info.sessionName.end(), info.sessionName.begin(), [] (unsigned char c) -> unsigned char { return std::tolower(c); });
			info.state = sessions[i].State;
			mSessions.emplace_back(info);
		}
		WTSFreeMemoryEx(WTSTypeSessionInfoLevel1, sessions, count);
	}

	const std::vector<SessionInfo> &WTSSession::getSessions() {
		return mSessions;
	}

	const SessionInfo *WTSSession::getCurrentSession() {
		if (mCurrentSession == nullptr) {
			for (auto &session : mSessions){
				if (session.state == WTSActive) {
					mCurrentSession = &session;
				}
			}
		}
		return mCurrentSession;
	}
	
	void WTSSession::connect(const SessionInfo &session) {
		auto res = WTSConnectSessionA(LOGONID_CURRENT, session.sessionId, "", true);
		if (!res) {
			LOGGER_ERROR("Failed to connect to session " + session.sessionName);
			LOGGER_ERROR("Error reason: " + std::to_string(GetLastError()));
		}
	}

	void WTSSession::disconnect(const SessionInfo &session) {
		auto res = WTSDisconnectSession(WTS_CURRENT_SERVER_HANDLE, session.sessionId, true);
		if (!res) {
			LOGGER_ERROR("Failed to disconnect from session " + session.sessionName);
			LOGGER_ERROR("Error reason: " + std::to_string(GetLastError()));
		}
	}

	void WTSSession::disconnectCurrent() {
		auto s = getCurrentSession();
		if (s != nullptr)
			disconnect(*s);
	}
};