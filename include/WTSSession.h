#pragma once

#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <wtsapi32.h>

namespace WTSService {

	struct SessionInfo
	{
		uint32_t sessionId;
		std::string sessionName;
		WTS_CONNECTSTATE_CLASS state;
	};

	class WTSSession {
	private:
		std::vector<SessionInfo> mSessions;
		const SessionInfo *mCurrentSession = nullptr;
	public:
		WTSSession();
		WTSSession(const WTSSession &session) = delete;
		WTSSession(const WTSSession &&session) = delete;
		~WTSSession() = default;
		WTSSession &operator=(const WTSSession &session) = delete;
		WTSSession &operator=(const WTSSession &&session) = delete;
		const std::vector<SessionInfo> &getSessions();
		const SessionInfo *getCurrentSession();
		void connect(const SessionInfo &session);
		void disconnect(const SessionInfo &session);
		void disconnectCurrent();
		inline size_t getSessionCount() {
			return mSessions.size();
		};
	};
}