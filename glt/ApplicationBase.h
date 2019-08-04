#pragma once

namespace glt
{
	class CApplicationBase
	{
	public:
		CApplicationBase();
		virtual ~CApplicationBase();

		void run();

	protected:
		bool _initV();
		void _updateV(float vDeltaTime);
		void _destroyV();
	};
}