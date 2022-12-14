// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
//+---------------------------------------------------------------------------
//
//  File:       oletls.h
//

//
//  Purpose:    manage thread local storage for OLE
//
//  Notes:      The gTlsIndex is initialized at process attach time.
//              The per-thread data is allocated in CoInitialize in
//              single-threaded apartments or on first use in
//              multi-threaded apartments.
//
//----------------------------------------------------------------------------

#ifndef _OLETLS_H_
#define _OLETLS_H_

/**
#ifndef FEATURE_COMINTEROP_APARTMENT_SUPPORT
#error FEATURE_COMINTEROP_APARTMENT_SUPPORT is required for this file
#endif // FEATURE_COMINTEROP_APARTMENT_SUPPORT
*/
//+---------------------------------------------------------------------------
//
// forward declarations (in order to avoid type casting when accessing
// data members of the SOleTlsData structure).
//
//+---------------------------------------------------------------------------

/**
class  CAptCallCtrl;                        // see callctrl.hxx
class  CSrvCallState;                       // see callctrl.hxx
class  CObjServer;                          // see sobjact.hxx
class  CSmAllocator;                        // see stg\h\smalloc.hxx
class  CMessageCall;                        // see call.hxx
class  CClientCall;                         // see call.hxx
class  CAsyncCall;                          // see call.hxx
class  CClipDataObject;                     // see ole232\clipbrd\clipdata.h
class  CSurrogatedObjectList;               // see com\inc\comsrgt.hxx
class  CCtxCall;                            // see PSTable.hxx
class  CPolicySet;                          // see PSTable.hxx
class  CObjectContext;                      // see context.hxx
class  CComApartment;                       // see aprtmnt.hxx
*/

//+-------------------------------------------------------------------
//
//  Struct:     CallEntry
//
//  Synopsis:   Call Table Entry.
//
//+-------------------------------------------------------------------
typedef struct _tagCallEntry {
    void  *pNext;        // ptr to next entry
    void  *pvObject;     // Entry object
} CallEntry;

struct LockEntry;
// RWLock state inside TLS
typedef struct _tagLockEntry {
    struct LockEntry *pNext;    // next entry
    struct LockEntry *pPrev;    // prev entry
    LONG dwULockID;
    LONG dwLLockID;         // owning lock
    WORD wReaderLevel;      // reader nesting level
} LockEntry;

//+---------------------------------------------------------------------------
//
//  Enum:       OLETLSFLAGS
//
//  Synopsys:   bit values for dwFlags field of SOleTlsData. If you just want
//              to store a BOOL in TLS, use this enum and the dwFlag field.
//
//+---------------------------------------------------------------------------
typedef enum tagOLETLSFLAGS
{
    OLETLS_LOCALTID             = 0x01,   // This TID is in the current process.
    OLETLS_UUIDINITIALIZED      = 0x02,   // This Logical thread is init'd.
    OLETLS_INTHREADDETACH       = 0x04,   // This is in thread detach. Needed
                                          // due to NT's special thread detach
                                          // rules.
    OLETLS_CHANNELTHREADINITIALZED = 0x08,// This channel has been init'd
    OLETLS_WOWTHREAD            = 0x10,   // This thread is a 16-bit WOW thread.
    OLETLS_THREADUNINITIALIZING = 0x20,   // This thread is in CoUninitialize.
    OLETLS_DISABLE_OLE1DDE      = 0x40,   // This thread can't use a DDE window.
    OLETLS_APARTMENTTHREADED    = 0x80,   // This is an STA apartment thread
    OLETLS_MULTITHREADED        = 0x100,  // This is an MTA apartment thread
    OLETLS_IMPERSONATING        = 0x200,  // This thread is impersonating
    OLETLS_DISABLE_EVENTLOGGER  = 0x400,  // Prevent recursion in event logger
    OLETLS_INNEUTRALAPT         = 0x800,  // This thread is in the NTA
    OLETLS_DISPATCHTHREAD       = 0x1000, // This is a dispatch thread
    OLETLS_HOSTTHREAD           = 0x2000, // This is a host thread
    OLETLS_ALLOWCOINIT          = 0x4000, // This thread allows inits
    OLETLS_PENDINGUNINIT        = 0x8000, // This thread has pending uninit
    OLETLS_FIRSTMTAINIT         = 0x10000,// First thread to attempt an MTA init
    OLETLS_FIRSTNTAINIT         = 0x20000,// First thread to attempt an NTA init
    OLETLS_APTINITIALIZING      = 0x40000 // Apartment Object is initializing
}  OLETLSFLAGS;


//+---------------------------------------------------------------------------
//
//  Structure:  SOleTlsData
//
//  Synopsis:   structure holding per thread state needed by OLE32
//
//+---------------------------------------------------------------------------
typedef struct tagSOleTlsData
{
#if !defined(_CHICAGO_)
    // Docfile multiple allocator support
    void               *pvThreadBase;       // per thread base pointer
    void               *pSmAllocator;       // per thread docfile allocator, CSmAllocator
#endif
    DWORD               dwApartmentID;      // Per thread "process ID"
    DWORD               dwFlags;            // see OLETLSFLAGS above

    LONG                TlsMapIndex;        // index in the global TLSMap
    void              **ppTlsSlot;          // Back pointer to the thread tls slot
    DWORD               cComInits;          // number of per-thread inits
    DWORD               cOleInits;          // number of per-thread OLE inits

    DWORD               cCalls;             // number of outstanding calls
    void               *pCallInfo;          // channel call info, CMessageCall
    void               *pFreeAsyncCall;     // ptr to available call object for this thread, CAsyncCall
    void               *pFreeClientCall;    // ptr to available call object for this thread, CClientCall

    void               *pObjServer;         // Activation Server Object for this apartment, CObjServer
    DWORD               dwTIDCaller;        // TID of current calling app
    void     *pCurrentCtx;        // Current context, CObjectContext
    void     *pEmptyCtx;          // Empty context, CObjectContext

    void     *pNativeCtx;         // Native context, CObjectContext
    void      *pNativeApt;         // Native apartment for the thread, CComApartment
    void           *pCallContext;       // call context object, IUnknown
    void           *pCtxCall;           // Context call object, CCtxCall

    void         *pPS;                // Policy set, CPolicySet
    PVOID               pvPendingCallsFront;// Per Apt pending async calls
    PVOID               pvPendingCallsBack;
    void       *pCallCtrl;          // call control for RPC for this apartment, CAptCallCtrl

    void      *pTopSCS;            // top server-side callctrl state, CSrvCallState
    void     *pMsgFilter;         // temp storage for App MsgFilter, IMessageFilter
    HWND                hwndSTA;            // STA server window same as poxid->hServerSTA
                                            // ...needed on Win95 before oxid registration
    LONG                cORPCNestingLevel;  // call nesting level (DBG only)

    DWORD               cDebugData;         // count of bytes of debug data in call
    ULONG               cPreRegOidsAvail;   // count of server-side OIDs avail
    unsigned hyper     *pPreRegOids;        // ptr to array of pre-reg OIDs

    UUID                LogicalThreadId;    // current logical thread id

    HANDLE              hThread;            // Thread handle used for cancel
    HANDLE              hRevert;            // Token before first impersonate.
    void           *pAsyncRelease;      // Controlling unknown for async release, IUnknown
    // DDE data
    HWND                hwndDdeServer;      // Per thread Common DDE server

    HWND                hwndDdeClient;      // Per thread Common DDE client
    ULONG               cServeDdeObjects;   // non-zero if objects DDE should serve
    // ClassCache data
    LPVOID              pSTALSvrsFront;     // Chain of LServers registers in this thread if STA
    // upper layer data
    HWND                hwndClip;           // Clipboard window

    void         *pDataObjClip;      // Current Clipboard DataObject, IDataObject
    DWORD               dwClipSeqNum;       // Clipboard Sequence # for the above DataObject
    DWORD               fIsClipWrapper;     // Did we hand out the wrapper Clipboard DataObject?
    void            *punkState;         // Per thread "state" object, IUnknown
    // cancel data
    DWORD              cCallCancellation;   // count of CoEnableCallCancellation
    // async sends data
    DWORD              cAsyncSends;         // count of async sends outstanding

    void*           pAsyncCallList;   // async calls outstanding, CAsyncCall
    void *pSurrogateList;  // Objects in the surrogate, CSurrogatedObjectList

    LockEntry             lockEntry;        // Locks currently held by the thread
    CallEntry             CallEntry;        // client-side call chain for this thread

#ifdef WX86OLE
    void           *punkStateWx86;      // Per thread "state" object for Wx86, IUnknown
#endif
    void               *pDragCursors;       // Per thread drag cursor table.

#ifdef _CHICAGO_
    LPVOID              pWcstokContext;     // Scan context for wcstok
#endif

    void           *punkError;          // Per thread error object, IUnknown
    ULONG               cbErrorData;        // Maximum size of error data.

#if(_WIN32_WINNT >= 0x0500)
    void           *punkActiveXSafetyProvider; // IUnknown
#endif //(_WIN32_WINNT >= 0x0500)

#if DBG==1
    LONG                cTraceNestingLevel; // call nesting level for OLETRACE
#endif

} SOleTlsData;

//+---------------------------------------------------------------------------
//
//  Structure:  SOleTlsData
//
//  Synopsis:   structure holding per thread state needed by OLE32
//
//+---------------------------------------------------------------------------
typedef struct tagSOleTlsData2
{
    // jsimmons 5/23/2001
    // Alert Alert:  nefarious folks (eg, URT) are looking in our TLS at
    // various stuff.   They expect that pCurrentCtx will be at a certain
    // offset from the beginning of the tls struct. So don't add, delete, or 
    // move any members within this block.

/////////////////////////////////////////////////////////////////////////////////////////
// ********* BEGIN "NO MUCKING AROUND" BLOCK ********* 
/////////////////////////////////////////////////////////////////////////////////////////
    // Docfile multiple allocator support
    void               *pvThreadBase;       // per thread base pointer
    void       *pSmAllocator;       // per thread docfile allocator

    DWORD               dwApartmentID;      // Per thread "process ID"
    DWORD               dwFlags;            // see OLETLSFLAGS above

    LONG                TlsMapIndex;        // index in the global TLSMap
    void              **ppTlsSlot;          // Back pointer to the thread tls slot
    DWORD               cComInits;          // number of per-thread inits
    DWORD               cOleInits;          // number of per-thread OLE inits

    DWORD               cCalls;             // number of outstanding calls
    void       *pCallInfo;          // channel call info
    void         *pFreeAsyncCall;     // ptr to available call object for this thread.
    void        *pFreeClientCall;    // ptr to available call object for this thread.

    void         *pObjServer;         // Activation Server Object for this apartment.
    DWORD               dwTIDCaller;        // TID of current calling app
    void     *pCurrentCtx;        // Current context
/////////////////////////////////////////////////////////////////////////////////////////
//  ********* END "NO MUCKING AROUND" BLOCK ********* 
/////////////////////////////////////////////////////////////////////////////////////////

    void     *pEmptyCtx;          // Empty context

    void     *pNativeCtx;         // Native context
    ULONGLONG           ContextId;          // Uniquely identifies the current context
    void      *pNativeApt;         // Native apartment for the thread.
    void           *pCallContext;       // call context object
    void           *pCtxCall;           // Context call object

    void         *pPS;                // Policy set
    PVOID               pvPendingCallsFront;// Per Apt pending async calls
    PVOID               pvPendingCallsBack;
    void       *pCallCtrl;          // call control for RPC for this apartment

    void      *pTopSCS;            // top server-side callctrl state
    void     *pMsgFilter;         // temp storage for App MsgFilter
    HWND                hwndSTA;            // STA server window same as poxid->hServerSTA
                                            // ...needed on Win95 before oxid registration
    LONG                cORPCNestingLevel;  // call nesting level (DBG only)

    DWORD               cDebugData;         // count of bytes of debug data in call

    UUID                LogicalThreadId;    // current logical thread id

    HANDLE              hThread;            // Thread handle used for cancel
    HANDLE              hRevert;            // Token before first impersonate.
    void           *pAsyncRelease;      // Controlling unknown for async release
    // DDE data
    HWND                hwndDdeServer;      // Per thread Common DDE server

    HWND                hwndDdeClient;      // Per thread Common DDE client
    ULONG               cServeDdeObjects;   // non-zero if objects DDE should serve
    // ClassCache data
    LPVOID              pSTALSvrsFront;     // Chain of LServers registers in this thread if STA
    // upper layer data
    HWND                hwndClip;           // Clipboard window

    void         *pDataObjClip;      // Current Clipboard DataObject
    DWORD               dwClipSeqNum;       // Clipboard Sequence # for the above DataObject
    DWORD               fIsClipWrapper;     // Did we hand out the wrapper Clipboard DataObject?
    void            *punkState;         // Per thread "state" object
    // cancel data
    DWORD              cCallCancellation;   // count of CoEnableCallCancellation
    // async sends data
    DWORD              cAsyncSends;         // count of async sends outstanding

    void*           pAsyncCallList;   // async calls outstanding
    void *pSurrogateList;  // Objects in the surrogate

    LockEntry             lockEntry;        // Locks currently held by the thread
    CallEntry             CallEntry;        // client-side call chain for this thread

#ifdef WX86OLE
    void           *punkStateWx86;      // Per thread "state" object for Wx86
#endif
    void               *pDragCursors;       // Per thread drag cursor table.

    void           *punkError;          // Per thread error object.
    ULONG               cbErrorData;        // Maximum size of error data.

    void           *punkActiveXSafetyProvider;

#if DBG==1
    LONG                cTraceNestingLevel; // call nesting level for OLETRACE
#endif

    void* pContextStack;

} SOleTlsData2;

#ifdef INITGUID
#include "initguid.h"
#endif

#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

DEFINE_OLEGUID(IID_IStdIdentity,        0x0000001bL, 0, 0);
DEFINE_OLEGUID(IID_IStdWrapper,         0x000001caL, 0, 0);

#endif // _OLETLS_H_
