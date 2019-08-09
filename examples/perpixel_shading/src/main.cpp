#include "MyApplication.h"

int main()
{
	CMyApplication App;
	App.setWindowWidth(1600);
	App.setWindowHeight(900);
	App.setWindowTitle("Per-pixel Shading Demo");
	App.run();

	return 0;
}