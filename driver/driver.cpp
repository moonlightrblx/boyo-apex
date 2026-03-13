#include <ntifs.h>
#include <windef.h>
#include <intrin.h>

UNICODE_STRING DriverName, SymbolicLinkName;

typedef struct _SystemBigpoolEntry {
    PVOID VirtualAddress;
    ULONG_PTR NonPaged : 1;
    ULONG_PTR SizeInBytes;
    UCHAR Tag[4];
} SystemBigpoolEntry, * PSystemBigpoolEntry;

typedef struct _SystemBigpoolInformation {
    ULONG Count;
    SystemBigpoolEntry AllocatedInfo[1];
} SystemBigpoolInformation, * PSystemBigpoolInformation;

typedef enum _SystemInformationClass {
    SystemBigpoolInformationClass = 0x42,
} SystemInformationClass;

extern "C" NTSTATUS NTAPI IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
extern "C" PVOID NTAPI PsGetProcessSectionBaseAddress(PEPROCESS Process);
extern "C" NTSTATUS NTAPI ZwQuerySystemInformation(SystemInformationClass systemInformationClass, PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength);
extern "C" NTSTATUS NTAPI RtlGetVersion(PRTL_OSVERSIONINFOW lpVersionInformation);
extern "C" NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
    PUNICODE_STRING ObjectName,
    ULONG Attributes,
    PACCESS_STATE AccessState,
    ACCESS_MASK DesiredAccess,
    POBJECT_TYPE ObjectType,
    KPROCESSOR_MODE AccessMode,
    PVOID ParseContext,
    PVOID* Object
);
extern "C" NTSYSAPI POBJECT_TYPE* IoDriverObjectType;

#define BlasterRead CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1363, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define BlasterBase CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1369, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define BlasterMouse CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define BlasterSecurity 0x75C8BA6

#define Win1803 17134
#define Win1809 17763
#define Win1903 18362
#define Win1909 18363
#define Win2004 19041
#define Win20H2 19569
#define Win21H1 20180
#define win11_22h2 22621

#define PageOffsetSize 12
static const UINT64 PageMask = (~0xfull << 8) & 0xfffffffffull;


#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_LEFT_BUTTON_DOWN   0x0001
#define MOUSE_LEFT_BUTTON_UP     0x0002
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004
#define MOUSE_RIGHT_BUTTON_UP    0x0008
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010
#define MOUSE_MIDDLE_BUTTON_UP   0x0020

typedef struct _ReadWriteRequest {
    INT32 Security;
    INT32 ProcessId;
    ULONGLONG Address;
    ULONGLONG Buffer;
    ULONGLONG Size;
    BOOLEAN Write;
} ReadWriteRequest, * PReadWriteRequest;

typedef struct _BaseAddressRequest {
    INT32 Security;
    INT32 ProcessId;
    ULONGLONG* Address;
} BaseAddressRequest, * PBaseAddressRequest;


#pragma warning(disable : 4201)
typedef struct _MOUSE_INPUT_DATA {
    USHORT UnitId;
    USHORT Flags;
    union {
        ULONG Buttons;
        struct {
            USHORT ButtonFlags;
            USHORT ButtonData;
        };
    };
    ULONG RawButtons;
    LONG LastX;
    LONG LastY;
    ULONG ExtraInformation;
} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;

typedef VOID(*MouseClassServiceCallbackFn)(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);

typedef struct _MOUSE_OBJECT {
    PDEVICE_OBJECT mouse_device;
    MouseClassServiceCallbackFn service_callback;
    BOOLEAN use_mouse;
} MOUSE_OBJECT, * PMOUSE_OBJECT;

typedef struct _KMOUSE_REQUEST {
    long x;
    long y;
    unsigned char button_flags;
} KMOUSE_REQUEST, * PKMOUSE_REQUEST;

MOUSE_OBJECT gMouseObject = { 0 };

extern "C" {
    VOID MouseClassServiceCallback(PDEVICE_OBJECT DeviceObject, PMOUSE_INPUT_DATA InputDataStart, PMOUSE_INPUT_DATA InputDataEnd, PULONG InputDataConsumed);
}

NTSTATUS ReadPhysicalMemory(PVOID TargetAddress, PVOID Buffer, SIZE_T Size, SIZE_T* BytesRead) {
    MM_COPY_ADDRESS CopyAddress = { 0 };
    CopyAddress.PhysicalAddress.QuadPart = (LONGLONG)TargetAddress;
    return MmCopyMemory(Buffer, CopyAddress, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}

NTSTATUS write(PVOID target_address, PVOID buffer, SIZE_T size, SIZE_T* bytes_read) {
    if (!target_address)
        return STATUS_UNSUCCESSFUL;

    PHYSICAL_ADDRESS AddrToWrite = { 0 };
    AddrToWrite.QuadPart = LONGLONG(target_address);

    PVOID pmapped_mem = MmMapIoSpaceEx(AddrToWrite, size, PAGE_READWRITE);

    if (!pmapped_mem)
        return STATUS_UNSUCCESSFUL;

    memcpy(pmapped_mem, buffer, size);

    *bytes_read = size;
    MmUnmapIoSpace(pmapped_mem, size);
    return STATUS_SUCCESS;
}

INT32 GetWindowsVersion() {
    RTL_OSVERSIONINFOW VersionInfo = { 0 };
    RtlGetVersion(&VersionInfo);
    switch (VersionInfo.dwBuildNumber) {
    case Win1803: case Win1809: return 0x0278;
    case Win1903: case Win1909: return 0x0280;
    case Win2004: case Win20H2: case Win21H1: return 0x0388;
    case win11_22h2: return 0x0390;
    default: return 0x0388;
    }
}

UINT64 GetProcessCr3(PEPROCESS Process) {
    if (!Process) return 0;
    uintptr_t process_dirbase = *(uintptr_t*)((UINT8*)Process + 0x28);
    if (process_dirbase == 0) {
        ULONG user_diroffset = GetWindowsVersion();
        process_dirbase = *(uintptr_t*)((UINT8*)Process + user_diroffset);
    }
    if ((process_dirbase >> 0x38) == 0x40) {
        uintptr_t SavedDirBase = 0;
        KAPC_STATE apc_state{};
        KeStackAttachProcess(Process, &apc_state);
        SavedDirBase = __readcr3();
        KeUnstackDetachProcess(&apc_state);
        return SavedDirBase;
    }
    return process_dirbase;
}

UINT64 TranslateLinearAddress(UINT64 DirectoryTableBase, UINT64 VirtualAddress) {
    DirectoryTableBase &= ~0xf;

    UINT64 PageOffset = VirtualAddress & ~(~0ul << PageOffsetSize);
    UINT64 PteIndex = ((VirtualAddress >> 12) & 0x1ffll);
    UINT64 PtIndex = ((VirtualAddress >> 21) & 0x1ffll);
    UINT64 PdIndex = ((VirtualAddress >> 30) & 0x1ffll);
    UINT64 PdpIndex = ((VirtualAddress >> 39) & 0x1ffll);

    SIZE_T ReadSize = 0;
    UINT64 PdpEntry = 0;
    ReadPhysicalMemory(PVOID(DirectoryTableBase + 8 * PdpIndex), &PdpEntry, sizeof(PdpEntry), &ReadSize);
    if (~PdpEntry & 1) return 0;

    UINT64 PdEntry = 0;
    ReadPhysicalMemory(PVOID((PdpEntry & PageMask) + 8 * PdIndex), &PdEntry, sizeof(PdEntry), &ReadSize);
    if (~PdEntry & 1) return 0;

    if (PdEntry & 0x80)
        return (PdEntry & (~0ull << 42 >> 12)) + (VirtualAddress & ~(~0ull << 30));

    UINT64 PtEntry = 0;
    ReadPhysicalMemory(PVOID((PdEntry & PageMask) + 8 * PtIndex), &PtEntry, sizeof(PtEntry), &ReadSize);
    if (~PtEntry & 1) return 0;

    if (PtEntry & 0x80)
        return (PtEntry & PageMask) + (VirtualAddress & ~(~0ull << 21));

    VirtualAddress = 0;
    ReadPhysicalMemory(PVOID((PtEntry & PageMask) + 8 * PteIndex), &VirtualAddress, sizeof(VirtualAddress), &ReadSize);
    VirtualAddress &= PageMask;

    if (!VirtualAddress) return 0;

    return VirtualAddress + PageOffset;
}

ULONG64 FindMin(INT32 A, SIZE_T B) {
    INT32 BInt = (INT32)B;
    return (((A) < (BInt)) ? (A) : (BInt));
}


BOOLEAN __forceinline mouse_open() {
    if (gMouseObject.use_mouse == 0) {
        UNICODE_STRING class_string;
        RtlInitUnicodeString(&class_string, L"\\Driver\\MouClass");

        PDRIVER_OBJECT class_driver_object = NULL;
        NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
        if (!NT_SUCCESS(status)) {
            gMouseObject.use_mouse = 0;
            return FALSE;
        }

        UNICODE_STRING hid_string;
        RtlInitUnicodeString(&hid_string, L"\\Driver\\MouHID");

        PDRIVER_OBJECT hid_driver_object = NULL;
        status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
        if (!NT_SUCCESS(status)) {
            if (class_driver_object)
                ObfDereferenceObject(class_driver_object);
            gMouseObject.use_mouse = 0;
            return FALSE;
        }

        PVOID class_driver_base = NULL;
        PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
        while (hid_device_object && !gMouseObject.service_callback) {
            PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
            while (class_device_object && !gMouseObject.service_callback) {
                if (!class_device_object->NextDevice && !gMouseObject.mouse_device)
                    gMouseObject.mouse_device = class_device_object;

                PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->DeviceExtension;
                ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->DeviceObjectExtension - (ULONG_PTR)hid_device_object->DeviceExtension) / 4;
                class_driver_base = class_driver_object->DriverStart;
                for (ULONG_PTR i = 0; i < device_ext_size; i++) {
                    if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object) {
                        gMouseObject.service_callback = (MouseClassServiceCallbackFn)(device_extension[i + 1]);
                        break;
                    }
                }
                class_device_object = class_device_object->NextDevice;
            }
            hid_device_object = hid_device_object->AttachedDevice;
        }

        if (!gMouseObject.mouse_device) {
            PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
            while (target_device_object) {
                if (!target_device_object->NextDevice) {
                    gMouseObject.mouse_device = target_device_object;
                    break;
                }
                target_device_object = target_device_object->NextDevice;
            }
        }

        ObfDereferenceObject(class_driver_object);
        ObfDereferenceObject(hid_driver_object);

        if (gMouseObject.mouse_device && gMouseObject.service_callback)
            gMouseObject.use_mouse = 1;
    }

    return gMouseObject.mouse_device && gMouseObject.service_callback;
}

VOID __forceinline mouse_move(long x, long y, unsigned short button_flags) {
    KIRQL irql;
    ULONG input_data;
    MOUSE_INPUT_DATA mid = { 0 };
    mid.LastX = x;
    mid.LastY = y;
    mid.ButtonFlags = button_flags;

    if (!mouse_open())
        return;

    KeRaiseIrql(DISPATCH_LEVEL, &irql);
    gMouseObject.service_callback(gMouseObject.mouse_device, &mid, &mid + 1, &input_data);
    KeLowerIrql(irql);
}

NTSTATUS HandleMouseRequest(PKMOUSE_REQUEST Request) {
    if (!Request)
        return STATUS_INVALID_PARAMETER;

    mouse_move(Request->x, Request->y, Request->button_flags);
    return STATUS_SUCCESS;
}

NTSTATUS HandleReadRequest(PReadWriteRequest Request) {
    if (Request->Security != BlasterSecurity || !Request->ProcessId)
        return STATUS_UNSUCCESSFUL;

    PEPROCESS Process = NULL;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)Request->ProcessId, &Process)))
        return STATUS_UNSUCCESSFUL;

    ULONGLONG ProcessBase = GetProcessCr3(Process);
    ObDereferenceObject(Process);

    SIZE_T Offset = 0;
    SIZE_T TotalSize = Request->Size;

    INT64 PhysicalAddress = TranslateLinearAddress(ProcessBase, (ULONG64)Request->Address + Offset);
    if (!PhysicalAddress)
        return STATUS_UNSUCCESSFUL;

    ULONG64 FinalSize = FindMin(PAGE_SIZE - (PhysicalAddress & 0xFFF), TotalSize);
    SIZE_T BytesRead = 0;

    return ReadPhysicalMemory(PVOID(PhysicalAddress), (PVOID)((ULONG64)Request->Buffer + Offset), FinalSize, &BytesRead);
}

NTSTATUS HandleWriteRequest(PReadWriteRequest Request) {
    if (Request->Security != BlasterSecurity || !Request->ProcessId)
        return STATUS_UNSUCCESSFUL;

    PEPROCESS Process = NULL;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)Request->ProcessId, &Process)))
        return STATUS_UNSUCCESSFUL;

    ULONGLONG ProcessBase = GetProcessCr3(Process);
    ObDereferenceObject(Process);

    SIZE_T Offset = 0;
    SIZE_T TotalSize = Request->Size;

    INT64 PhysicalAddress = TranslateLinearAddress(ProcessBase, (ULONG64)Request->Address + Offset);
    if (!PhysicalAddress)
        return STATUS_UNSUCCESSFUL;

    ULONG64 FinalSize = FindMin(PAGE_SIZE - (PhysicalAddress & 0xFFF), TotalSize);
    SIZE_T BytesWritten = 0;

    return write(PVOID(PhysicalAddress), PVOID((ULONG64)Request->Buffer + Offset), FinalSize, &BytesWritten);
}

NTSTATUS HandleBaseAddressRequest(PBaseAddressRequest Request) {
    if (Request->Security != BlasterSecurity || !Request->ProcessId)
        return STATUS_UNSUCCESSFUL;

    PEPROCESS Process = NULL;
    if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)Request->ProcessId, &Process)))
        return STATUS_UNSUCCESSFUL;

    ULONGLONG ImageBase = (ULONGLONG)PsGetProcessSectionBaseAddress(Process);
    ObDereferenceObject(Process);
    if (!ImageBase)
        return STATUS_UNSUCCESSFUL;

    RtlCopyMemory(Request->Address, &ImageBase, sizeof(ImageBase));
    return STATUS_SUCCESS;
}

NTSTATUS IoControlHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    NTSTATUS Status = {};
    ULONG BytesReturned = 0;
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

    ULONG IoControlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG InputBufferLength = Stack->Parameters.DeviceIoControl.InputBufferLength;

    if (IoControlCode == BlasterRead) {
        if (InputBufferLength == sizeof(ReadWriteRequest)) {
            PReadWriteRequest Request = (PReadWriteRequest)(Irp->AssociatedIrp.SystemBuffer);
            Status = Request->Write ? HandleWriteRequest(Request) : HandleReadRequest(Request);
            BytesReturned = sizeof(ReadWriteRequest);
        }
        else {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }
    else if (IoControlCode == BlasterBase) {
        if (InputBufferLength == sizeof(BaseAddressRequest)) {
            PBaseAddressRequest Request = (PBaseAddressRequest)(Irp->AssociatedIrp.SystemBuffer);
            Status = HandleBaseAddressRequest(Request);
            BytesReturned = sizeof(BaseAddressRequest);
        }
        else {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }
    else if (IoControlCode == BlasterMouse) {
        if (InputBufferLength == sizeof(KMOUSE_REQUEST)) {
            PKMOUSE_REQUEST Request = (PKMOUSE_REQUEST)(Irp->AssociatedIrp.SystemBuffer);
            Status = HandleMouseRequest(Request);
            BytesReturned = sizeof(KMOUSE_REQUEST);
        }
        else {
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = BytesReturned;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

NTSTATUS UnsupportedDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

NTSTATUS DispatchHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);
    switch (Stack->MajorFunction) {
    case IRP_MJ_CREATE:
    case IRP_MJ_CLOSE:
        break;
    default:
        break;
    }
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

void UnloadDriver(PDRIVER_OBJECT DriverObject) {
    IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS InitializeDriver(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;

   
    RtlZeroMemory(&gMouseObject, sizeof(MOUSE_OBJECT));

    RtlInitUnicodeString(&DriverName, L"\\Device\\{SteamStore}");
    RtlInitUnicodeString(&SymbolicLinkName, L"\\DosDevices\\{SteamStore}");

    Status = IoCreateDevice(DriverObject, 0, &DriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
    if (!NT_SUCCESS(Status)) return Status;

    Status = IoCreateSymbolicLink(&SymbolicLinkName, &DriverName);
    if (!NT_SUCCESS(Status)) {
        IoDeleteDevice(DeviceObject);
        return Status;
    }

    for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        DriverObject->MajorFunction[i] = &UnsupportedDispatch;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = &DispatchHandler;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = &DispatchHandler;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &IoControlHandler;
    DriverObject->DriverUnload = &UnloadDriver;

    DeviceObject->Flags |= DO_BUFFERED_IO;
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    return Status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    return IoCreateDriver(NULL, &InitializeDriver);
}
