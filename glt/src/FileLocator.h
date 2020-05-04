#pragma once
#include <set>
#include <vector>
#include <string>
#include <mutex>
#include "Common.h"
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CFileLocator
	{
	public:
		~CFileLocator();
		_SINGLETON(CFileLocator);

		void addFileSearchPath(const std::string& vPath);
		void addFileSearchPath(const std::vector<std::string>& vPathSet);

		std::string locateFile(const std::string& vFileName);

#ifdef _DEBUG
		size_t getNumSearchPath() const { return m_SearchPathSet.size(); }
#endif

	private:
		CFileLocator();

		std::set<std::string> m_SearchPathSet;
		std::mutex m_Mutex;
	};
}