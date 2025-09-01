// NoScreen Example By Developer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "kernel.h"

int main ( )
{
    if ( !kernel->IsLoaded ( ) ) {
        std::cerr << "Could not find driver. Make sure NoScreen is loaded." << std::endl;
        Sleep ( 2500 );
        return 1;
    }
    std::cout << "Driver found successfully." << std::endl;

    HWND hwnd = FindWindowA ( NULL , "Untitled - Notepad" );
    if ( !hwnd ) {
        std::cerr << "Target window not found. Make sure Notepad is open with the correct title." << std::endl;
        Sleep ( 2500 );
        return 1;
    }
    std::cout << "Window handle found: 0x" << hwnd << std::endl;

    NTSTATUS status = kernel->ProtectWindow ( hwnd , WDA_NONE ); // WDA_NONE UNHIDES IT // WDA_EXCLUDEFROMCAPTURE HIDES IT.
    if ( status == STATUS_SUCCESS ) {
        std::cout << "Window has been hidden from capture successfully." << std::endl;
    }
    else {
        std::cerr << "Failed to hide window. NTSTATUS: 0x" << std::hex << status << std::endl;
    }

    Sleep ( 2500 );
    return 0;
}
