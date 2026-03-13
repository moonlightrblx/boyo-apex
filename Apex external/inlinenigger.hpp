#pragma once
#include <Windows.h>
#include <cstring>

// Error codes
#define IS_ADDRESS_NOT_FOUND -1
#define IS_CALLBACK_KILL_FAILURE -2
#define IS_INTEGRITY_STUB_FAILURE -3
#define IS_MODULE_NOT_FOUND -4
#define IS_ALLOCATION_FAILURE -5
#define IS_INIT_NOT_APPLIED -6
#define IS_HEAP_FAILURE -7
#define IS_SUCCESS 0

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// XOR obfuscation
#define XOR_KEY 0x5A
inline void xor_obfuscate(UCHAR* data, size_t size, bool encrypt) {
    for (size_t i = 0; i < size; i++) data[i] ^= XOR_KEY;
}

inline HINSTANCE hSubsystemInstances[2] = { nullptr, nullptr };

class inline_syscall {
public:
    inline_syscall();
    ~inline_syscall();
    void unload();
    void callback();

    void set_error(int error_code) { last_error = error_code; }
    int get_error() const { return last_error; }
    bool is_init() const { return initialized; }
    UCHAR* get_stub() const { return syscall_stub; }

    template <typename returnType, typename ...args>
    returnType invoke(LPCSTR ServiceName, args... arguments);

private:
    int last_error = IS_INIT_NOT_APPLIED;
    bool initialized = false;
    UCHAR* syscall_stub = nullptr;
    SIZE_T stub_size = 32;
    HANDLE process_heap = nullptr;
    CRITICAL_SECTION cs;
    bool stub_obfuscated = false;

    typedef NTSTATUS(__stdcall* pNtSetInformationProcess)(
        HANDLE ProcessHandle,
        PROCESS_INFORMATION_CLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength
        );

    struct PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION {
        ULONG Version;
        ULONG Reserved;
        PVOID Callback;
    };

    inline bool adjust_stub_permissions();
};

// Constructor
inline inline_syscall::inline_syscall() {
    process_heap = GetProcessHeap();
    if (!process_heap) {
        last_error = IS_HEAP_FAILURE;
        return;
    }

    hSubsystemInstances[0] = LoadLibraryA("ntdll.dll");
    hSubsystemInstances[1] = LoadLibraryA("win32u.dll");

    for (UINT i = 0; i < sizeof hSubsystemInstances / sizeof HINSTANCE; i++) {
        if (hSubsystemInstances[i] == nullptr) {
            last_error = IS_MODULE_NOT_FOUND;
            return;
        }
    }

    syscall_stub = (UCHAR*)HeapAlloc(process_heap, HEAP_ZERO_MEMORY, stub_size);
    if (!syscall_stub) {
        last_error = IS_ALLOCATION_FAILURE;
        return;
    }

#ifdef _M_X64
    UCHAR stub_template[] = { 0x4C,0x8B,0xD1,0xB8,0x00,0x00,0x00,0x00,0x0F,0x05,0xC3 };
    memcpy(syscall_stub, stub_template, sizeof(stub_template));
    memset(syscall_stub + sizeof(stub_template), 0x90, stub_size - sizeof(stub_template)); // NOP padding
#else
    // Add x86 stub if needed
#endif

    InitializeCriticalSection(&cs);

    if (!adjust_stub_permissions()) {
        last_error = IS_HEAP_FAILURE;
        HeapFree(process_heap, 0, syscall_stub);
        syscall_stub = nullptr;
        return;
    }

    xor_obfuscate(syscall_stub, stub_size, true);
    stub_obfuscated = true;

    callback();
    if (last_error != IS_SUCCESS)
        return;

    last_error = IS_SUCCESS;
    initialized = true;
}

// Destructor
inline inline_syscall::~inline_syscall() {
    DeleteCriticalSection(&cs);
    unload();
}

inline void inline_syscall::unload() {
    if (syscall_stub) {
        SecureZeroMemory(syscall_stub, stub_size);
        HeapFree(process_heap, 0, syscall_stub);
        syscall_stub = nullptr;
    }
}

inline bool inline_syscall::adjust_stub_permissions() {
    DWORD oldProtect;
    return VirtualProtect(syscall_stub, stub_size, PAGE_EXECUTE_READWRITE, &oldProtect) != 0;
}

inline UCHAR* GetExportByName(HINSTANCE module, LPCSTR name) {
    BYTE* base = (BYTE*)module;
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)base;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

    IMAGE_NT_HEADERS* ntHeaders = (IMAGE_NT_HEADERS*)(base + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return nullptr;
    if (ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) return nullptr;

    IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)(base + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    DWORD* nameArray = (DWORD*)(base + exportDir->AddressOfNames);
    WORD* ordinalArray = (WORD*)(base + exportDir->AddressOfNameOrdinals);
    DWORD* funcArray = (DWORD*)(base + exportDir->AddressOfFunctions);

    for (DWORD i = 0; i < exportDir->NumberOfNames; i++) {
        char* funcName = (char*)(base + nameArray[i]);
        if (strcmp(funcName, name) == 0) {
            WORD ordinal = ordinalArray[i];
            DWORD funcRVA = funcArray[ordinal];
            if (funcRVA >= ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
                funcRVA < ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size) {
                return nullptr; // Forwarded export
            }
            return base + funcRVA;
        }
    }
    return nullptr;
}

inline void inline_syscall::callback() {
    pNtSetInformationProcess NtSet = (pNtSetInformationProcess)GetExportByName(hSubsystemInstances[0], "NtSetInformationProcess");
    if (!NtSet) {
        set_error(IS_ADDRESS_NOT_FOUND);
        return;
    }

    PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION info = { 0, 0, nullptr };
    NTSTATUS status = NtSet(GetCurrentProcess(), (PROCESS_INFORMATION_CLASS)40, &info, sizeof(info));
    if (!NT_SUCCESS(status)) {
        set_error(IS_CALLBACK_KILL_FAILURE);
        return;
    }

    set_error(IS_SUCCESS);
}

template <typename returnType, typename ...args>
inline returnType inline_syscall::invoke(LPCSTR ServiceName, args... arguments) {
    if (!initialized) {
        set_error(IS_INIT_NOT_APPLIED);
        return returnType{};
    }

    EnterCriticalSection(&cs);

    returnType result{};
    bool success = false;

    if (stub_obfuscated) {
        xor_obfuscate(syscall_stub, stub_size, false);
        stub_obfuscated = false;
    }

    for (UINT i = 0; i < sizeof hSubsystemInstances / sizeof(HINSTANCE); ++i) {
        UCHAR* FunctionAddress = GetExportByName(hSubsystemInstances[i], ServiceName);
        if (FunctionAddress != nullptr) {
#ifdef _M_X64
            if (*(UINT*)FunctionAddress != 0xB8D18B4C) {
                set_error(IS_INTEGRITY_STUB_FAILURE);
                result = returnType{};
                break;
            }

            INT SystemCallIndex = *(INT*)(FunctionAddress + 4);
            memcpy(syscall_stub + 4, &SystemCallIndex, sizeof(INT));
            if (i == 1) { // win32u.dll special copy
                memcpy(syscall_stub, FunctionAddress, 11);
            }
#else
            // Add x86 integrity check and stub build if needed
#endif

            typedef returnType __stdcall NtFunction(args...);
            NtFunction* Function = (NtFunction*)syscall_stub;
            result = Function(arguments...);
            set_error(IS_SUCCESS);
            success = true;
            break;
        }
    }

    if (!success) {
        set_error(IS_MODULE_NOT_FOUND);
        result = returnType{};
    }

    xor_obfuscate(syscall_stub, stub_size, true);
    stub_obfuscated = true;

    LeaveCriticalSection(&cs);
    return result;
}