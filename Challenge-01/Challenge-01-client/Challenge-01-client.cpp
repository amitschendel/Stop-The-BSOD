#include <Windows.h>
#include <iostream>
#include "../Challange-01/Common.h"

int Error(const std::string& message);

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		std::cout << "Usage: Challenge-01 <threadid> <priority>\n" << std::endl;
		return 0;
	}

	auto hDevice = CreateFile(L"\\\\.\\StopTheBsod", GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (hDevice == INVALID_HANDLE_VALUE)
		return Error("Failed to open device");

	ThreadData data;
	data.ThreadId = atoi(argv[1]);
	data.Priority = atoi(argv[2]);

	DWORD returned;
	auto success = DeviceIoControl(hDevice,
		IOCTL_PRIORITY_BOOSTER_SET_PRIORITY, // control code
		&data, sizeof(data), // input buffer and length
		nullptr, 0, // output buffer and length
		&returned, nullptr);

	if (success)
		std::cout << "Priority change succeeded!\n" << std::endl;
	else
		Error("Priority change failed!");

	CloseHandle(hDevice);

}

int Error(const std::string& message) {
	std::cout << message  + "\n" << std::endl;
	return GetLastError();
}
