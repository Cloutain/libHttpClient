// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#pragma once
#include "pch.h"
#include <wrl.h>
#include <winhttp.h>
#include "utils.h"
#include "uri.h"

NAMESPACE_XBOX_HTTP_CLIENT_BEGIN

enum msg_body_type
{
    no_body,
    content_length_chunked,
    transfer_encoding_chunked
};

class win32_cs
{
public:
    win32_cs() { InitializeCriticalSection(&m_cs); }
    ~win32_cs() { DeleteCriticalSection(&m_cs); }

    void lock() { EnterCriticalSection(&m_cs); }
    void unlock() { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};

class win32_cs_autolock
{
public:
    win32_cs_autolock(win32_cs* pCS)
        : m_pCS(pCS)
    {
        m_pCS->lock();
        //HC_TRACE_INFORMATION(HTTPCLIENT, "win32_cs_autolock locked [ID %lu]", GetCurrentThreadId());
    }

    ~win32_cs_autolock()
    {
        //HC_TRACE_INFORMATION(HTTPCLIENT, "win32_cs_autolock unlocking [ID %lu]", GetCurrentThreadId());
        m_pCS->unlock();
    }

private:
    win32_cs* m_pCS;
};

class winhttp_http_task : public xbox::httpclient::hc_task
{
public:
    winhttp_http_task(
        _Inout_ XAsyncBlock* asyncBlock,
        _In_ hc_call_handle_t call
        );
    ~winhttp_http_task();

    void perform_async();

private:
    static HRESULT query_header_length(_In_ hc_call_handle_t call, _In_ HINTERNET hRequestHandle, _In_ DWORD header, _Out_ DWORD* pLength);
    static uint32_t parse_status_code(
        _In_ hc_call_handle_t call,
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext);

    static void read_next_response_chunk(_In_ winhttp_http_task* pRequestContext, DWORD bytesRead);
    static void _multiple_segment_write_data(_In_ winhttp_http_task* pRequestContext);

    static void parse_headers_string(_In_ hc_call_handle_t call, _In_ wchar_t* headersStr);

    static void callback_status_request_error(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ void* statusInfo);

    static void callback_status_sendrequest_complete(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ void* statusInfo);

    static void callback_status_write_complete(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ void* statusInfo);

    static void callback_status_headers_available(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ void* statusInfo);

    static void callback_status_data_available(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ void* statusInfo);

    static void callback_status_read_complete(
        _In_ HINTERNET hRequestHandle,
        _In_ winhttp_http_task* pRequestContext,
        _In_ DWORD statusInfoLength);

    HRESULT send(_In_ const xbox::httpclient::Uri& cUri);

    HRESULT connect(_In_ const xbox::httpclient::Uri& cUri);

    void complete_task(_In_ HRESULT translatedHR);

    void complete_task(_In_ HRESULT translatedHR, uint32_t platformSpecificError);

    void get_proxy_name(
        _Out_ DWORD* pAccessType,
        _Out_ const wchar_t** pwProxyName
        );

    void set_autodiscover_proxy(_In_ const xbox::httpclient::Uri& cUri);

    static void get_proxy_info(
        _In_ WINHTTP_PROXY_INFO* pInfo,
        _In_ bool* pProxyInfoRequired,
        _In_ const xbox::httpclient::Uri& cUri);

    static void CALLBACK completion_callback(
        HINTERNET hRequestHandle,
        DWORD_PTR context,
        DWORD statusCode,
        _In_ void* statusInfo,
        DWORD statusInfoLength);

    hc_call_handle_t m_call;
    XAsyncBlock* m_asyncBlock;

    HINTERNET m_hSession;
    HINTERNET m_hConnection;
    HINTERNET m_hRequest;
    msg_body_type m_requestBodyType;
    uint64_t m_requestBodyRemainingToWrite;
    uint64_t m_requestBodyOffset;
    http_internal_vector<uint8_t> m_responseBuffer;

    xbox::httpclient::Uri m_proxyUri;
    http_internal_wstring m_wProxyName;
    proxy_type m_proxyType;
    win32_cs m_lock;
};


NAMESPACE_XBOX_HTTP_CLIENT_END
