#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <ntstatus.h>

struct g_driver_t
{
    HANDLE driver_handle = INVALID_HANDLE_VALUE;
    NTSTATUS driver_status = STATUS_NOT_FOUND;

    static constexpr ULONG io_offset = 0x2056;
    static constexpr ULONG io_device_type = 0x3138;
    static constexpr ULONG ioctl_hide_window = CTL_CODE ( io_device_type , io_offset + 0x0010 , METHOD_BUFFERED , FILE_READ_ACCESS | FILE_WRITE_ACCESS );

    struct window_protection_request
    {
        uint32_t affinity_value;
        uint64_t window_handle;
    };

    bool IsLoaded ( )
    {
        driver_handle = CreateFileW (
            L"\\\\.\\{*Developer*}" ,
            GENERIC_READ | GENERIC_WRITE ,
            FILE_SHARE_READ | FILE_SHARE_WRITE ,
            nullptr ,
            OPEN_EXISTING ,
            0 ,
            nullptr
        );
        return driver_handle != INVALID_HANDLE_VALUE;
    }

    NTSTATUS Send ( ULONG ioctl_code , LPVOID io , DWORD size )
    {
        if ( driver_handle == INVALID_HANDLE_VALUE )
            return STATUS_DEVICE_DOES_NOT_EXIST;

        if ( !DeviceIoControl ( driver_handle , ioctl_code , io , size , nullptr , 0 , nullptr , nullptr ) )
            return STATUS_UNSUCCESSFUL;

        return STATUS_SUCCESS;
    }

    NTSTATUS ProtectWindow ( HWND window_handle , uint32_t affinity_value )
    {
        window_protection_request req = { 0 };
        req.window_handle = reinterpret_cast< uint64_t >( window_handle );
        req.affinity_value = affinity_value;

        return Send ( ioctl_hide_window , &req , sizeof ( req ) );
    }
};

inline g_driver_t* kernel = new g_driver_t ( );
