#pragma once

///////////////////////////////////////////////////////////
// Utility Macros
///////////////////////////////////////////////////////////
#define to_rva(address, offset) address + (int32_t)((*(int32_t*)(address + offset) + offset) + sizeof(int32_t))

///////////////////////////////////////////////////////////
// Convert ANSI string to UNICODE_STRING
///////////////////////////////////////////////////////////
UNICODE_STRING ansi_to_unicode ( const char* str )
{
    UNICODE_STRING unicode;
    ANSI_STRING ansi_str;

    RtlInitAnsiString ( &ansi_str , str );
    RtlAnsiStringToUnicodeString ( &unicode , &ansi_str , TRUE );

    return unicode;
}

///////////////////////////////////////////////////////////
// Get address of a system routine by name
///////////////////////////////////////////////////////////
PVOID get_kernel_proc_address ( const char* system_routine_name )
{
    UNICODE_STRING name;
    ANSI_STRING ansi_str;

    RtlInitAnsiString ( &ansi_str , system_routine_name );
    RtlAnsiStringToUnicodeString ( &name , &ansi_str , TRUE );

    return MmGetSystemRoutineAddress ( &name );
}

///////////////////////////////////////////////////////////
// Retrieve base address of a loaded module
///////////////////////////////////////////////////////////
PVOID get_module_base ( const char* module_name )
{
    PLIST_ENTRY ps_loaded_module_list = PsLoadedModuleList;
    if ( !ps_loaded_module_list )
        return nullptr;

    UNICODE_STRING name = ansi_to_unicode ( module_name );

    for ( PLIST_ENTRY link = ps_loaded_module_list; link != ps_loaded_module_list->Blink; link = link->Flink )
    {
        PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD ( link , LDR_DATA_TABLE_ENTRY , InLoadOrderModuleList );

        if ( RtlEqualUnicodeString ( &entry->BaseDllName , &name , TRUE ) )
            return entry->DllBase;
    }

    return nullptr;
}

///////////////////////////////////////////////////////////
// Get exported routine from a module
///////////////////////////////////////////////////////////
PVOID get_system_base_export ( const char* module_name , LPCSTR routine_name )
{
    PVOID lp_module = get_module_base ( module_name );
    if ( !lp_module )
        return nullptr;

    return RtlFindExportedRoutineByName ( lp_module , routine_name );
}

///////////////////////////////////////////////////////////
// Get LDR entry by module name
///////////////////////////////////////////////////////////
PKLDR_DATA_TABLE_ENTRY get_ldr_data_by_name ( const char* szmodule )
{
    PKLDR_DATA_TABLE_ENTRY ldr_entry = nullptr;
    UNICODE_STRING mod = ansi_to_unicode ( szmodule );

    PLIST_ENTRY ps_loaded_module_list = PsLoadedModuleList;
    if ( !ps_loaded_module_list )
        return nullptr;

    auto current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( ps_loaded_module_list->Flink );

    while ( reinterpret_cast< PLIST_ENTRY >( current_ldr_entry ) != ps_loaded_module_list )
    {
        // Compare module names case-insensitively
        if ( !RtlCompareUnicodeString ( &current_ldr_entry->BaseDllName , &mod , TRUE ) )
        {
            ldr_entry = current_ldr_entry;
            break;
        }

        current_ldr_entry = reinterpret_cast< PKLDR_DATA_TABLE_ENTRY >( current_ldr_entry->InLoadOrderLinks.Flink );
    }

    return ldr_entry;
}

///////////////////////////////////////////////////////////
// Case-insensitive string compare helper
///////////////////////////////////////////////////////////
template <typename str_type , typename str_type_2>
__forceinline bool crt_strcmp ( str_type str , str_type_2 in_str , bool two )
{
#define to_lower(c_char) ((c_char >= 'A' && c_char <= 'Z') ? (c_char + 32) : c_char)

    if ( !str || !in_str )
        return false;

    wchar_t c1 , c2;
    do
    {
        c1 = *str++;
        c2 = *in_str++;
        c1 = to_lower ( c1 );
        c2 = to_lower ( c2 );

        if ( !c1 && ( two ? !c2 : 1 ) )
            return true;

    } while ( c1 == c2 );

    return false;
}

///////////////////////////////////////////////////////////
// Get IMAGE_SECTION_HEADER by section name
///////////////////////////////////////////////////////////
PIMAGE_SECTION_HEADER get_section_header ( const uintptr_t image_base , const char* section_name )
{
    if ( !image_base || !section_name )
        return nullptr;

    auto pimage_dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( image_base );
    auto pimage_nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS64 >( image_base + pimage_dos_header->e_lfanew );

    auto psection = reinterpret_cast< PIMAGE_SECTION_HEADER >( pimage_nt_headers + 1 );

    PIMAGE_SECTION_HEADER psection_hdr = nullptr;
    const auto number_of_sections = pimage_nt_headers->FileHeader.NumberOfSections;

    for ( auto i = 0; i < number_of_sections; ++i )
    {
        if ( crt_strcmp ( reinterpret_cast< const char* > ( psection->Name ) , section_name , false ) )
        {
            psection_hdr = psection;
            break;
        }

        ++psection;
    }

    return psection_hdr;
}

///////////////////////////////////////////////////////////
// Pattern matching helpers
///////////////////////////////////////////////////////////
bool data_compare ( const char* pdata , const char* bmask , const char* szmask )
{
    for ( ; *szmask; ++szmask , ++pdata , ++bmask )
    {
        if ( *szmask == 'x' && *pdata != *bmask )
            return false;
    }
    return !*szmask;
}

uintptr_t find_pattern ( const uintptr_t base , const size_t size , const char* bmask , const char* szmask )
{
    for ( size_t i = 0; i < size; ++i )
        if ( data_compare ( reinterpret_cast< const char* > ( base + i ) , bmask , szmask ) )
            return base + i;

    return 0;
}

uintptr_t find_pattern_page_km ( const char* szmodule , const char* szsection , const char* bmask , const char* szmask )
{
    if ( !szmodule || !szsection || !bmask || !szmask )
        return 0;

    auto* pldr_entry = get_ldr_data_by_name ( szmodule );
    if ( !pldr_entry )
        return 0;

    auto module_base = reinterpret_cast< uintptr_t >( pldr_entry->DllBase );
    auto* psection = get_section_header ( module_base , szsection );

    return psection ? find_pattern ( module_base + psection->VirtualAddress , psection->Misc.VirtualSize , bmask , szmask ) : 0;
}

///////////////////////////////////////////////////////////
// Initialize driver-specific addresses
///////////////////////////////////////////////////////////
NTSTATUS init_function ( )
{
    // Search for gre_protect_sprite_content in win32kfull.sys
    auto gre_protect_sprite_content_address = reinterpret_cast< PVOID >(
        find_pattern_page_km ( "win32kfull.sys" , ".text" ,
            "\xE8\x00\x00\x00\x00\x8B\xF8\x85\xC0\x75\x0E" , "x????xxxxxx" ) );

    // Windows 11 24H2 fallback
    if ( !gre_protect_sprite_content_address )
    {
        gre_protect_sprite_content_address = reinterpret_cast< PVOID >(
            find_pattern_page_km ( "win32kfull.sys" , ".text" ,
                "\xE8\x00\x00\x00\x00\x8B\xD8\x85\xC0\x75\x0E" , "x????xxxxxx" ) );
    }

    if ( !gre_protect_sprite_content_address )
        return STATUS_INVALID_ADDRESS;

    // Convert relative call to absolute address
    gre_protect_sprite_content_address = reinterpret_cast< PVOID >(
        to_rva ( reinterpret_cast< uintptr_t >( gre_protect_sprite_content_address ) , 1 ) );

    *( PVOID* ) &gre_protect_sprite_content = gre_protect_sprite_content_address;

    return STATUS_SUCCESS;
}

///////////////////////////////////////////////////////////
// Protect sprite content wrapper
///////////////////////////////////////////////////////////
NTSTATUS protect_sprite_content_fn ( pprotect_sprite_content req )
{
    // Calls the underlying gre_protect_sprite_content kernel routine
    return gre_protect_sprite_content ( 0 , req->window_handle , 1 , req->value )
        ? STATUS_SUCCESS
        : STATUS_UNSUCCESSFUL;
}
