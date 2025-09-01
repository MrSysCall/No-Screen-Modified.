#include "library/globals.h"

NTSTATUS drv_main ( PDRIVER_OBJECT driver_obj , PUNICODE_STRING registry_path )
{
    UNREFERENCED_PARAMETER ( registry_path ); // Registry path is unused in this driver

    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT dev_obj = nullptr;

    // Initialize any driver-specific resources
    status = init_function ( );
    if ( !NT_SUCCESS ( status ) )
        return status;

    // Device and symbolic link names
    UNICODE_STRING dev_name = ansi_to_unicode ( "\\Device\\{*Developer*}" );    
    UNICODE_STRING sym_link = ansi_to_unicode ( "\\DosDevices\\{*Developer*}" );

    // Create device object
    status = IoCreateDevice (
        driver_obj ,       // Driver object
        0 ,                // No device extension needed
        &dev_name ,        // Device name
        file_device_mirrore , // Device type
        0x00000100 ,       // Device characteristics
        FALSE ,            // Not exclusive
        &dev_obj          // Out: pointer to device object
    );
    if ( !NT_SUCCESS ( status ) )
        return status;

    // Create symbolic link for user-mode access
    status = IoCreateSymbolicLink ( &sym_link , &dev_name );
    if ( !NT_SUCCESS ( status ) )
        return status;

    // Enable buffered I/O for the device
    SetFlag ( dev_obj->Flags , DO_BUFFERED_IO );

    // Initialize all IRP major functions to unsupported
    for ( int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ )
        driver_obj->MajorFunction [ i ] = unsupported_io;

    // Set supported IRP handlers
    driver_obj->MajorFunction [ IRP_MJ_CREATE ] = create_io;          // Open handle
    driver_obj->MajorFunction [ IRP_MJ_CLOSE ] = close_io;            // Close handle
    driver_obj->MajorFunction [ IRP_MJ_DEVICE_CONTROL ] = ctl_io;     // IOCTL handling

    // Optional: set driver unload routine if needed
    driver_obj->DriverUnload = nullptr;

    // Clear device initializing flag so it becomes operational
    ClearFlag ( dev_obj->Flags , DO_DEVICE_INITIALIZING );

    return status;
}


// drivers entry point

NTSTATUS entry_point ( PVOID a1 , PVOID a2 )
{

    return IoCreateDriver (
        &ansi_to_unicode ( "\\Driver\\*Developer*" ) ,
        &drv_main
    );
}