#pragma once
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CAtomicCounterBuffer
	{
	public:
		CAtomicCounterBuffer(unsigned int vBindUnit);
		~CAtomicCounterBuffer();

		void reset();

	private:
		unsigned int m_ObjectID = 0;
		unsigned int m_BindUnit = 0;
	};
}