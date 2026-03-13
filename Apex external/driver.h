#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>

uintptr_t virtualaddy;

#define IoRead CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1363, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IoBase CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1369, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IoMouse CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define BlasterSecurity 0x75C8BA6


#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_LEFT_BUTTON_DOWN   0x0001
#define MOUSE_LEFT_BUTTON_UP     0x0002
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004
#define MOUSE_RIGHT_BUTTON_UP    0x0008
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010
#define MOUSE_MIDDLE_BUTTON_UP   0x0020

typedef struct _rw {
	INT32 security;
	INT32 process_id;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
	BOOLEAN write;
} rw, * prw;

typedef struct _ba {
	INT32 security;
	INT32 process_id;
	ULONGLONG* address;
} ba, * pba;

typedef struct _ga {
	INT32 security;
	ULONGLONG* address;
} ga, * pga;

typedef struct _mouse_request {
	long x;
	long y;
	unsigned char button_flags;
} mouse_request, * pmouse_request;

namespace drv {
	HANDLE DriverHandle;
	INT32 ProcessIdentifier;

	bool Init() {
		DriverHandle = CreateFileW((L"\\\\.\\{SteamStore}"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (!DriverHandle || (DriverHandle == INVALID_HANDLE_VALUE))
			return false;

		return true;
	}

	void ReadPhysical(PVOID address, PVOID buffer, DWORD size) {
		_rw arguments = { 0 };

		arguments.security = BlasterSecurity;
		arguments.address = (ULONGLONG)address;
		arguments.buffer = (ULONGLONG)buffer;
		arguments.size = size;
		arguments.process_id = ProcessIdentifier;
		arguments.write = FALSE;

		DeviceIoControl(DriverHandle, IoRead, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}

	void WritePhysical(PVOID address, PVOID buffer, DWORD size) {
		_rw arguments = { 0 };

		arguments.security = BlasterSecurity;
		arguments.address = (ULONGLONG)address;
		arguments.buffer = (ULONGLONG)buffer;
		arguments.size = size;
		arguments.process_id = ProcessIdentifier;
		arguments.write = TRUE;

		DeviceIoControl(DriverHandle, IoRead, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
	}

	uintptr_t GetBaseAddress() {
		uintptr_t image_address = { NULL };
		_ba arguments = { NULL };

		arguments.security = BlasterSecurity;
		arguments.process_id = ProcessIdentifier;
		arguments.address = (ULONGLONG*)&image_address;

		DeviceIoControl(DriverHandle, IoBase, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

		return image_address;
	}


	bool MoveMouse(long x, long y, unsigned char button_flags = 0) {
		if (!DriverHandle || DriverHandle == INVALID_HANDLE_VALUE) {
			if (!Init()) {
				return false;
			}
		}

		mouse_request arguments = { 0 };
		arguments.x = x;
		arguments.y = y;
		arguments.button_flags = button_flags;

		DWORD bytes_returned = 0;
		return DeviceIoControl(DriverHandle, IoMouse, &arguments, sizeof(arguments), nullptr, NULL, &bytes_returned, NULL);
	}

	bool LeftClick() {
		bool result = MoveMouse(0, 0, MOUSE_LEFT_BUTTON_DOWN);
		Sleep(10);
		result &= MoveMouse(0, 0, MOUSE_LEFT_BUTTON_UP);
		return result;
	}

	bool RightClick() {
		bool result = MoveMouse(0, 0, MOUSE_RIGHT_BUTTON_DOWN);
		Sleep(10);
		result &= MoveMouse(0, 0, MOUSE_RIGHT_BUTTON_UP);
		return result;
	}

	bool MiddleClick() {
		bool result = MoveMouse(0, 0, MOUSE_MIDDLE_BUTTON_DOWN);
		Sleep(10);
		result &= MoveMouse(0, 0, MOUSE_MIDDLE_BUTTON_UP);
		return result;
	}

	bool LeftClickDown() {
		return MoveMouse(0, 0, MOUSE_LEFT_BUTTON_DOWN);
	}

	bool LeftClickUp() {
		return MoveMouse(0, 0, MOUSE_LEFT_BUTTON_UP);
	}

	bool RightClickDown() {
		return MoveMouse(0, 0, MOUSE_RIGHT_BUTTON_DOWN);
	}

	bool RightClickUp() {
		return MoveMouse(0, 0, MOUSE_RIGHT_BUTTON_UP);
	}

	bool MiddleClickDown() {
		return MoveMouse(0, 0, MOUSE_MIDDLE_BUTTON_DOWN);
	}

	bool MiddleClickUp() {
		return MoveMouse(0, 0, MOUSE_MIDDLE_BUTTON_UP);
	}

	INT32 FindProcessID(LPCTSTR process_name) {
		PROCESSENTRY32 pt;
		HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		pt.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hsnap, &pt)) {
			do {
				if (!lstrcmpi(pt.szExeFile, process_name)) {
					CloseHandle(hsnap);
					ProcessIdentifier = pt.th32ProcessID;
					return pt.th32ProcessID;
				}
			} while (Process32Next(hsnap, &pt));
		}
		CloseHandle(hsnap);

		return NULL;
	}
}

template <typename T>
T read(uint64_t address) {
	T buffer{ };
	drv::ReadPhysical((PVOID)address, &buffer, sizeof(T));
	return buffer;
}

template <typename T>
void write(uint64_t address, const T& value) {
	drv::WritePhysical((PVOID)address, (PVOID)&value, sizeof(T));
}

bool IsValid(const uint64_t address)
{
	if (address <= 0x400000 || address == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(address) == nullptr || address > 0x7FFFFFFFFFFFFFFF) {
		return false;
	}
	return true;
}

template<typename T>
bool ReadArray(uintptr_t address, T out[], size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		out[i] = read<T>(address + i * sizeof(T));
	}
	return true;
}

template<typename T>
bool ReadArray2(uint64_t address, T* out, size_t len)
{
	if (!drv::DriverHandle || drv::DriverHandle == INVALID_HANDLE_VALUE) {
		if (!drv::Init()) {
			return false;
		}
	}

	if (!out || len == 0) {
		return false;
	}

	for (size_t i = 0; i < len; ++i) {
		if (!IsValid(address + i * sizeof(T))) {
			return false;
		}
		out[i] = read<T>(address + i * sizeof(T));
	}
	return true;
}