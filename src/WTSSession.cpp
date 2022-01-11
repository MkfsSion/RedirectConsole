#include <algorithm>
#include <cctype>

#include <windows.h>
#include <winternl.h>

#include "Logger.h"
#include "WTSSession.h"


namespace WTSService {
    namespace {
        std::string GetLastErrorAsString() {
            //Get the error message ID, if any.
            DWORD errorMessageID = ::GetLastError();
            if(errorMessageID == 0) {
                return std::string(); //No error message has been recorded
            }

            LPSTR messageBuffer = nullptr;

            //Ask Win32 to give us the string version of that message ID.
            //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                         NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            //Copy the error message into a std::string.
            std::string message(messageBuffer, size);

            //Free the Win32's string's buffer.
            LocalFree(messageBuffer);

            return message;
        }
    }
	WTSSession::WTSSession() {
		DWORD count = 0;
        	PWTS_SESSION_INFOA sessions;
		auto result = WTSEnumerateSessionsA(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions, &count);
		if (!result || count == 0)
		{
			LOGGER_ERROR("Failed to get sessions information");
			LOGGER_ERROR("Error reason: " + GetLastErrorAsString());
		}
		LOGGER_DEBUG("Successfully get " + std::to_string(count) + " sessions");
		for (int i = 0; i < static_cast<int>(count); i++) {
			SessionInfo info;
			info.sessionId = sessions[i].SessionId;
			info.sessionName = sessions[i].pWinStationName;
			std::transform(info.sessionName.begin(), info.sessionName.end(), info.sessionName.begin(), [] (unsigned char c) -> unsigned char { return std::tolower(c); });
			info.state = sessions[i].State;
			mSessions.emplace_back(info);
		}
		WTSFreeMemory(sessions);
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
		auto res = WTSConnectSessionA(LOGONID_CURRENT, session.sessionId, const_cast<char*>(""), true);
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
