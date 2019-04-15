﻿/*
 * PROJECT:   M2-Team Common Library
 * FILE:      M2BaseHelpers.cpp
 * PURPOSE:   Implementation for the basic helper functions
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include "stdafx.h"

#include <Windows.h>

#include "M2BaseHelpers.h"

#include <string>

/**
 * Write formatted data to a string.
 *
 * @param Format Format-control string.
 * @param ... Optional arguments to be formatted.
 * @return A formatted string if successful, "N/A" otherwise.
 */
std::wstring M2FormatString(
    _In_z_ _Printf_format_string_ wchar_t const* const Format,
    ...)
{
    // Check the argument list.
    if (nullptr != Format)
    {
        va_list ArgList = nullptr;
        va_start(ArgList, Format);

        // Get the length of the format result.
        size_t nLength = static_cast<size_t>(_vscwprintf(Format, ArgList)) + 1;

        // Allocate for the format result.
        std::wstring Buffer(nLength + 1, L'\0');

        // Format the string.
        int nWritten = _vsnwprintf_s(
            &Buffer[0],
            Buffer.size(),
            nLength,
            Format,
            ArgList);

        va_end(ArgList);

        if (nWritten > 0)
        {
            // If succeed, resize to fit and return result.
            Buffer.resize(nWritten);
            return Buffer;
        }
    }

    // If failed, return "N/A".
    return L"N/A";
}

/**
 * Converts from the UTF-8 string to the UTF-16 string.
 *
 * @param UTF8String The UTF-8 string you want to convert.
 * @return A converted UTF-16 string.
 */
std::wstring M2MakeUTF16String(const std::string& UTF8String)
{
    std::wstring UTF16String;

    int UTF16StringLength = MultiByteToWideChar(
        CP_UTF8,
        0,
        UTF8String.data(),
        (int)UTF8String.size(),
        nullptr,
        0);
    if (UTF16StringLength > 0)
    {
        UTF16String.resize(UTF16StringLength);
        MultiByteToWideChar(
            CP_UTF8,
            0,
            UTF8String.data(),
            (int)UTF8String.size(),
            &UTF16String[0],
            UTF16StringLength);
    }

    return UTF16String;
}

/**
 * Converts from the UTF-16 string to the UTF-8 string.
 *
 * @param UTF16String The UTF-16 string you want to convert.
 * @return A converted UTF-8 string.
 */
std::string M2MakeUTF8String(const std::wstring& UTF16String)
{
    std::string UTF8String;

    int UTF8StringLength = WideCharToMultiByte(
        CP_UTF8,
        0,
        UTF16String.data(),
        (int)UTF16String.size(),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (UTF8StringLength > 0)
    {
        UTF8String.resize(UTF8StringLength);
        WideCharToMultiByte(
            CP_UTF8,
            0,
            UTF16String.data(),
            (int)UTF16String.size(),
            &UTF8String[0],
            UTF8StringLength,
            nullptr,
            nullptr);
    }

    return UTF8String;
}

/**
 * Retrieves the path of the executable file of the current process.
 *
 * @return If the function succeeds, the return value is the path of the
 *         executable file of the current process. If the function fails, the
 *         return value is an empty string.
 */
std::wstring M2GetCurrentProcessModulePath()
{
    std::wstring result(MAX_PATH, L'\0');
    GetModuleFileNameW(nullptr, &result[0], (DWORD)(result.capacity()));
    result.resize(wcslen(result.c_str()));
    return result;
}

/**
 * Parses a command line string and returns an array of the command line
 * arguments, along with a count of such arguments, in a way that is similar to
 * the standard C run-time.
 *
 * @param CommandLine A string that contains the full command line. If this
 *                    parameter is an empty string the function returns an
 *                    array with only one empty string.
 * @return An array of the command line arguments, along with a count of such
 *         arguments.
 */
std::vector<std::wstring> M2SpiltCommandLine(
    const std::wstring& CommandLine)
{
    // Initialize the SplitArguments.
    std::vector<std::wstring> SplitArguments;

    wchar_t c = L'\0';
    int copy_character;                   /* 1 = copy char to *args */
    unsigned numslash;              /* num of backslashes seen */

    std::wstring Buffer;
    Buffer.reserve(CommandLine.size());

    /* first scan the program name, copy it, and count the bytes */
    wchar_t* p = const_cast<wchar_t*>(CommandLine.c_str());

    // A quoted program name is handled here. The handling is much simpler than
    // for other arguments. Basically, whatever lies between the leading
    // double-quote and next one, or a terminal null character is simply
    // accepted. Fancier handling is not required because the program name must
    // be a legal NTFS/HPFS file name. Note that the double-quote characters are
    // not copied, nor do they contribute to character_count.
    bool InQuotes = false;
    do
    {
        if (*p == '"')
        {
            InQuotes = !InQuotes;
            c = *p++;
            continue;
        }

        // Copy character into argument:
        Buffer.push_back(*p);

        c = *p++;
    } while (c != '\0' && (InQuotes || (c != ' ' && c != '\t')));

    if (c == '\0')
    {
        p--;
    }
    else
    {
        Buffer.resize(Buffer.size() - 1);
    }

    // Save te argument.
    SplitArguments.push_back(Buffer);

    InQuotes = false;

    // Loop on each argument
    for (;;)
    {
        if (*p)
        {
            while (*p == ' ' || *p == '\t')
                ++p;
        }

        // End of arguments
        if (*p == '\0')
            break;

        // Initialize the argument buffer.
        Buffer.clear();

        // Loop through scanning one argument:
        for (;;)
        {
            copy_character = 1;

            // Rules: 2N backslashes + " ==> N backslashes and begin/end quote
            // 2N + 1 backslashes + " ==> N backslashes + literal " N
            // backslashes ==> N backslashes
            numslash = 0;

            while (*p == '\\')
            {
                // Count number of backslashes for use below
                ++p;
                ++numslash;
            }

            if (*p == '"')
            {
                // if 2N backslashes before, start/end quote, otherwise copy
                // literally:
                if (numslash % 2 == 0)
                {
                    if (InQuotes && p[1] == '"')
                    {
                        p++; // Double quote inside quoted string
                    }
                    else
                    {
                        // Skip first quote char and copy second:
                        copy_character = 0; // Don't copy quote
                        InQuotes = !InQuotes;
                    }
                }

                numslash /= 2;
            }

            // Copy slashes:
            while (numslash--)
            {
                Buffer.push_back(L'\\');
            }

            // If at end of arg, break loop:
            if (*p == '\0' || (!InQuotes && (*p == ' ' || *p == '\t')))
                break;

            // Copy character into argument:
            if (copy_character)
            {
                Buffer.push_back(*p);
            }

            ++p;
        }

        // Save te argument.
        SplitArguments.push_back(Buffer);
    }

    return SplitArguments;
}

/**
 * Parses a command line string and get more friendly result.
 *
 * @param CommandLine A string that contains the full command line. If this
 *                    parameter is an empty string the function returns an
 *                    array with only one empty string.
 * @param OptionPrefixes One or more of the prefixes of option we want to use.
 * @param OptionParameterSeparators One or more of the separators of option we
 *                                  want to use.
 * @param ApplicationName The application name.
 * @param OptionsAndParameters The options and parameters.
 * @param UnresolvedCommandLine The unresolved command line.
 */
void M2SpiltCommandLineEx(
    const std::wstring& CommandLine,
    const std::vector<std::wstring>& OptionPrefixes,
    const std::vector<std::wstring>& OptionParameterSeparators,
    std::wstring& ApplicationName,
    std::map<std::wstring, std::wstring>& OptionsAndParameters,
    std::wstring& UnresolvedCommandLine)
{
    ApplicationName.clear();
    OptionsAndParameters.clear();
    UnresolvedCommandLine.clear();

    size_t arg_size = 0;
    for (auto& SplitArgument : M2SpiltCommandLine(CommandLine))
    {
        // We need to process the application name at the beginning.
        if (ApplicationName.empty())
        {
            // For getting the unresolved command line, we need to cumulate
            // length which including spaces.
            arg_size += SplitArgument.size() + 1;

            // Save
            ApplicationName = SplitArgument;
        }
        else
        {
            bool IsOption = false;
            size_t OptionPrefixLength = 0;

            for (auto& OptionPrefix : OptionPrefixes)
            {
                if (0 == _wcsnicmp(
                    SplitArgument.c_str(),
                    OptionPrefix.c_str(),
                    OptionPrefix.size()))
                {
                    IsOption = true;
                    OptionPrefixLength = OptionPrefix.size();
                }
            }

            if (IsOption)
            {
                // For getting the unresolved command line, we need to cumulate
                // length which including spaces.
                arg_size += SplitArgument.size() + 1;

                // Get the option name and parameter.

                wchar_t* OptionStart = &SplitArgument[0] + OptionPrefixLength;
                wchar_t* ParameterStart = nullptr;

                for (auto& OptionParameterSeparator
                    : OptionParameterSeparators)
                {
                    wchar_t* Result = wcsstr(
                        OptionStart,
                        OptionParameterSeparator.c_str());
                    if (nullptr == Result)
                    {
                        continue;
                    }

                    Result[0] = L'\0';
                    ParameterStart = Result + OptionParameterSeparator.size();

                    break;
                }

                // Save
                OptionsAndParameters[(OptionStart ? OptionStart : L"")] =
                    (ParameterStart ? ParameterStart : L"");
            }
            else
            {
                // Get the approximate location of the unresolved command line.
                // We use "(arg_size - 1)" to ensure that the program path
                // without quotes can also correctly parse.
                wchar_t* search_start =
                    const_cast<wchar_t*>(CommandLine.c_str()) + (arg_size - 1);

                // Get the unresolved command line. Search for the beginning of
                // the first parameter delimiter called space and exclude the
                // first space by adding 1 to the result.
                wchar_t* command = wcsstr(search_start, L" ") + 1;

                // Omit the space. (Thanks to wzzw.)
                while (command && *command == L' ')
                {
                    ++command;
                }

                // Save
                if (command)
                {
                    UnresolvedCommandLine = command;
                }

                break;
            }
        }
    }
}

/**
 * Deletes an existing file, even the file have the readonly attribute.
 *
 * @param FileHandle The handle of the file to be deleted.. This handle must be
 *                   opened with the appropriate permissions for the requested
 *                   change. This handle should not be a pipe handle.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 * @remark The way to get a file handle for this operation:
 *         HANDLE hFile = CreateFileW(
 *             lpFileName,
 *             SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
 *             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
 *             nullptr,
 *             OPEN_EXISTING,
 *             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
 *             nullptr);
 */
HRESULT M2DeleteFileIgnoreReadonlyAttribute(
    _In_ HANDLE FileHandle)
{
    HRESULT hr = S_OK;
    DWORD OldAttribute = 0;

    // Save old attributes.
    hr = M2GetFileAttributes(
        FileHandle,
        &OldAttribute);
    if (!SUCCEEDED(hr)) return hr;

    // Remove readonly attribute.
    hr = M2SetFileAttributes(
        FileHandle,
        OldAttribute & (-1 ^ FILE_ATTRIBUTE_READONLY));
    if (!SUCCEEDED(hr)) return hr;

    // Delete the file.
    hr = M2DeleteFile(FileHandle);
    if (!SUCCEEDED(hr))
    {
        // Restore attributes if failed.
        hr = M2SetFileAttributes(
            FileHandle,
            OldAttribute);
    }

    return hr;
}

/**
 * Retrieves the amount of space that is allocated for the file.
 *
 * @param FileHandle A handle to the file that contains the information to be
 *                   retrieved. This handle should not be a pipe handle.
 * @param AllocationSize A pointer to a ULONGLONG value that receives the
 *                       amount of space that is allocated for the file, in
 *                       bytes.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 * @remark The way to get a file handle for this operation:
 *         HANDLE hFile = CreateFileW(
 *             lpFileName,
 *             GENERIC_READ | SYNCHRONIZE,
 *             FILE_SHARE_READ,
 *             nullptr,
 *             OPEN_EXISTING,
 *             FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
 *             nullptr);
 */
HRESULT M2GetFileAllocationSize(
    _In_ HANDLE FileHandle,
    _Out_ PULONGLONG AllocationSize)
{
    FILE_STANDARD_INFO StandardInfo;

    if (GetFileInformationByHandleEx(
        FileHandle,
        FileStandardInfo,
        &StandardInfo,
        sizeof(FILE_STANDARD_INFO)))
    {
        *AllocationSize = static_cast<ULONGLONG>(
            StandardInfo.AllocationSize.QuadPart);
        return S_OK;
    }
    else
    {
        *AllocationSize = 0;
        return M2GetLastHRESULTErrorKnownFailedCall();
    }
}

/**
 * Opens the access token associated with a process.
 *
 * @param TokenHandle A pointer to a handle that identifies the newly opened
 *                    access token when the function returns.
 * @param TokenSource The source information of access token associated with a
 *                    process.
 * @param DesiredAccess Specifies an access mask that specifies the requested
 *                      types of access to the access token. These requested
 *                      access types are compared with the discretionary access
 *                      control list (DACL) of the token to determine which
 *                      accesses are granted or denied.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 */
HRESULT M2OpenProcessToken(
    _Out_ PHANDLE TokenHandle,
    _In_ PM2_PROCESS_ACCESS_TOKEN_SOURCE TokenSource,
    _In_ DWORD DesiredAccess)
{
    *TokenHandle = INVALID_HANDLE_VALUE;

    if (!TokenSource)
        return __HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    HANDLE ProcessHandle = INVALID_HANDLE_VALUE;

    switch (TokenSource->Type)
    {
    case M2_PROCESS_TOKEN_SOURCE_TYPE::Current:
        ProcessHandle = GetCurrentProcess();
        break;
    case M2_PROCESS_TOKEN_SOURCE_TYPE::Handle:
        ProcessHandle = TokenSource->ProcessHandle;
        break;
    case M2_PROCESS_TOKEN_SOURCE_TYPE::ProcessId:
        ProcessHandle = OpenProcess(
            MAXIMUM_ALLOWED, FALSE, TokenSource->ProcessId);
        if (!ProcessHandle)
            return M2GetLastHRESULTErrorKnownFailedCall();
        break;
    default:
        return __HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if (OpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle))
        return S_OK;

    return M2GetLastHRESULTErrorKnownFailedCall();
}

/**
 * Allocates a block of memory from the default heap of the calling process.
 * The allocated memory will be initialized to zero. The allocated memory is
 * not movable.
 *
 * @param AllocatedMemoryBlock A pointer to the allocated memory block.
 * @param MemoryBlockSize The number of bytes to be allocated.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 */
HRESULT M2AllocMemory(
    _Out_ PVOID* AllocatedMemoryBlock,
    _In_ SIZE_T MemoryBlockSize)
{
    *AllocatedMemoryBlock = HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        MemoryBlockSize);

    return *AllocatedMemoryBlock
        ? S_OK
        : __HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
}

/**
 * Reallocates a block of memory from the default heap of the calling process.
 * If the reallocation request is for a larger size, the additional region of
 * memory beyond the original size be initialized to zero. This function
 * enables you to resize a memory block and change other memory block
 * properties. The allocated memory is not movable.
 *
 * @param NewAllocatedMemoryBlock A pointer to the allocated memory block.
 * @param OldAllocatedMemoryBlock A pointer to the block of memory that the
 *                                function reallocates. This pointer is
 *                                returned by an earlier call to the
 *                                M2AllocMemory or M2ReAllocMemory function. 
 * @param NewMemoryBlockSize The new size of the memory block, in bytes. A
 *                           memory block's size can be increased or decreased
 *                           by using this function.
 * @return HRESULT. If the function succeeds, the return value is S_OK. If the
 *         function fails, the original memory is not freed, and the original
 *         handle and pointer are still valid.
 */
HRESULT M2ReAllocMemory(
    _Out_ PVOID* NewAllocatedMemoryBlock,
    _In_ PVOID OldAllocatedMemoryBlock,
    _In_ SIZE_T NewMemoryBlockSize)
{
    *NewAllocatedMemoryBlock = HeapReAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        OldAllocatedMemoryBlock,
        NewMemoryBlockSize);

    return *NewAllocatedMemoryBlock
        ? S_OK
        : __HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
}

/**
 * Frees a memory block allocated from a heap by the M2AllocMemory and
 * M2ReAllocMemory function.
 *
 * @param AllocatedMemoryBlock A pointer to the memory block to be freed. This
 * pointer is returned by the M2AllocMemory or M2ReAllocMemory function. If
 * this pointer is nullptr, the behavior is undefined.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 */
HRESULT M2FreeMemory(
    _In_ PVOID AllocatedMemoryBlock)
{
    if (!HeapFree(GetProcessHeap(), 0, AllocatedMemoryBlock))
        return M2GetLastHRESULTErrorKnownFailedCall();

    return S_OK;
}

/**
 * Retrieves a specified type of information about an access token. The calling
 * process must have appropriate access rights to obtain the information.
 *
 * @param OutputInformation A pointer to a buffer the function fills with the
 *                          requested information. When you have finished using
 *                          the information, free it by calling the
 *                          M2FreeMemory function. You should also set the
 *                          pointer to NULL.
 * @param TokenHandle A handle to an access token from which information is
 *                    retrieved.
 * @param TokenInformationClass Specifies a value from the
 *                              TOKEN_INFORMATION_CLASS enumerated type to
 *                              identify the type of information the function
 *                              retrieves.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 * @remark For more information, see GetTokenInformation.
 */
HRESULT M2GetTokenInformation(
    _Out_ PVOID* OutputInformation,
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass)
{
    *OutputInformation = nullptr;

    DWORD Length = 0;

    HRESULT hr = M2GetTokenInformation(
        TokenHandle,
        TokenInformationClass,
        nullptr,
        0,
        &Length);
    if (hr == __HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
    {
        hr = M2AllocMemory(OutputInformation, Length);
        if (SUCCEEDED(hr))
        {
            hr = M2GetTokenInformation(
                TokenHandle,
                TokenInformationClass,
                *OutputInformation,
                Length,
                &Length);
            if (FAILED(hr))
            {
                hr = M2FreeMemory(*OutputInformation);
            }
        }
    }

    return hr;
}

/**
 * Expands environment-variable strings and replaces them with the values
 * defined for the current user.
 *
 * @param ExpandedString The expanded string.
 * @param VariableName The environment-variable string you need to expand.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
 */
HRESULT M2ExpandEnvironmentStrings(
    std::wstring& ExpandedString,
    const std::wstring& VariableName)
{
    HRESULT hr = S_OK;

    do
    {
        DWORD Length = ExpandEnvironmentStringsW(
            VariableName.c_str(),
            nullptr,
            0);
        if (0 == Length)
        {
            hr = M2GetLastHRESULTErrorKnownFailedCall();
            break;
        }

        ExpandedString.resize(Length - 1);

        Length = ExpandEnvironmentStringsW(
            VariableName.c_str(),
            &ExpandedString[0],
            static_cast<DWORD>(ExpandedString.size() + 1));
        if (0 == Length)
        {
            hr = M2GetLastHRESULTErrorKnownFailedCall();
            break;
        }
        if (ExpandedString.size() != Length - 1)
        {
            hr = E_UNEXPECTED;
            break;
        }

    } while (false);

    if (FAILED(hr))
    {
        ExpandedString.clear();
    }

    return hr;
}
