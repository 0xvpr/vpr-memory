/**
 * Created by:      VPR (0xvpr)
 * Created:         March 28th, 2024
 *
 * Updated by:      VPR (0xvpr)
 * Updated:         April 6th, 2024
 *
 * Description:     C99/C++20 Header only library for memory management in Windows.
 *
 * License:         MIT (c) VPR 2024
**/


#ifndef    VPR_DEVIATE_HEADER
#define    VPR_DEVIATE_HEADER



#ifndef    VC_EXTRA_LEAN
#define    VC_EXTRA_LEAN
#endif  // VC_EXTRA_LEAN
#include   <windows.h>



#if        !defined(__cplusplus)
#include   <stdbool.h>
#include   <stdint.h>

#else   // defined(__cplusplus)
#include   <cstdint>
#endif  // defined(__cplusplus)



#if        !defined(__cplusplus)
#define             _rel_jmp_      ( ((uint8_t)0xE9) )
#define             _rel_jmp_size_ ( ((sizeof(uint32_t)+1)) )
#define             _mov_rax_      ( (((uint16_t)(0x1234)) & 0xFF) == 0x34 ? 0xB848 : 0x48B8 )
#define             _jmp_rax_      ( (((uint16_t)(0x1234)) & 0xFF) == 0x34 ? 0xE0FF : 0xFFE0 )
/*#define           _jmp_rax_size_ ( (sizeof(uint16_t)) )*/
#else   // defined(__cplusplus)
constexpr uint8_t   _rel_jmp_      = (uint8_t)0xE9;
constexpr uint32_t  _rel_jmp_size_ = ((sizeof(uint32_t)+1));
constexpr uint16_t  _mov_rax_      = (((uint16_t)(0x1234)) & 0xFF) == 0x34 ? 0xB848 : 0x48B8;
constexpr uint16_t  _jmp_rax_      = (((uint16_t)(0x1234)) & 0xFF) == 0x34 ? 0xE0FF : 0xFFE0;
/*constexpr size_t  _jmp_rax_size_ = (sizeof(uint16_t));*/
#endif  // defined(__cplusplus)



////////////////////////////////////////////////////////////////////////////////
//                                 C API
////////////////////////////////////////////////////////////////////////////////


typedef struct __attribute__((packed)) _eax_jmp_data {
    uint16_t    mov_eax     : 16;
    uint32_t    address     : 32;
    uint16_t    jmp_eax     : 16;
} eax_jmp_data_t, *eax_jmp_data_ptr;

typedef struct __attribute__((packed)) _rax_jmp_data {
    uint16_t    mov_rax     : 16;
    uint64_t    address     : 64;
    uint16_t    jmp_rax     : 16;
} rax_jmp_data_t, *rax_jmp_data_ptr;

typedef struct __attribute__((packed)) _rel_jmp_data {
    uint8_t     rel_jmp     :  8;
    int32_t     address     : 32;
} rel_jmp_data_t, *rel_jmp_data_ptr;

typedef struct _original_data {
    char bytes[2 * sizeof(rax_jmp_data_t)];
} original_data, *original_data_ptr;

__forceinline
void set_rax_jmp_data(rax_jmp_data_ptr jmp_data, uint64_t address) {
    jmp_data->mov_rax = _mov_rax_;
    jmp_data->address = address;
    jmp_data->jmp_rax = _jmp_rax_;
}

__forceinline
void set_rel_jmp_data(rel_jmp_data_ptr rel_jmp_data, int32_t address) {
    rel_jmp_data->rel_jmp = _rel_jmp_;
    rel_jmp_data->address = address;
}

#if        !defined(__cplusplus)
/**
 * Direct Syscall of NtProtectVirtualMemory.
 *
 * @param:   HANDLE         process_handle,
 * @param:   PVOID*         base_address,
 * @param:   PSIZE_T        size_ptr,
 * @param:   DWORD          protect,
 * @param:   PDWORD         old_protect
 *
 * @return: uintptr_t       resolved_address
**/
NTSTATUS fNtProtectVirtualMemory( /* HANDLE  process_handle, */
                                  /* PVOID*  base_address,   */
                                  /* PSIZE_T size_ptr,       */
                                  /* DWORD   protect,        */
                                  /* PDWORD  old_protect     */
);
#endif  // !defined(__cplusplus)

#if        !defined(__cplusplus)
#ifndef VPR_DEVIATE_GLOBAL_C_INIT
#define VPR_DEVIATE_GLOBAL_C_INIT() \
NTSTATUS __declspec(naked) fNtProtectVirtualMemory( /* HANDLE  process_handle, */   \
                                                    /* PVOID*  base_address,   */   \
                                                    /* PSIZE_T size_ptr,       */   \
                                                    /* DWORD   protect,        */   \
                                                    /* PDWORD  old_protect     */ ) \
{ \
    __asm__ __volatile__( \
        ".byte 0x49, 0x89, 0xCA\n\t"              /* mov r10, rcx  */ \
        ".byte 0xB8, 0x50, 0x00, 0x00, 0x00\n\t"  /* mov eax, 0x50 */ \
        ".byte 0x0F, 0x05\n\t"                    /* syscall       */ \
        ".byte 0xC3"                              /* ret           */ \
    ); \
}
#endif
#else // defined(__cplusplus
/**
 * Direct Syscall of NtProtectVirtualMemory.
 *
 * @param:   HANDLE         process_handle,
 * @param:   PVOID*         base_address,
 * @param:   PSIZE_T        size_ptr,
 * @param:   DWORD          protect,
 * @param:   PDWORD         old_protect
 *
 * @return: uintptr_t       resolved_address
**/
inline
NTSTATUS __declspec(naked) fNtProtectVirtualMemory( /* HANDLE  process_handle, */
                                                    /* PVOID*  base_address,   */
                                                    /* PSIZE_T size_ptr,       */
                                                    /* DWORD   protect,        */
                                                    /* PDWORD  old_protect     */
                                                    ... // set variable args for C++ implementations
) {
    __asm__ __volatile__(
        ".byte 0x49, 0x89, 0xCA\n\t"              /* mov r10, rcx  */
        ".byte 0xB8, 0x50, 0x00, 0x00, 0x00\n\t"  /* mov eax, 0x50 */
        ".byte 0x0F, 0x05\n\t"                    /* syscall       */
        ".byte 0xC3"                              /* ret           */
    );
}
#endif


/**
 * Custom implementation of memcpy without using the C standard library.
 * Copies 'n' bytes from memory area 'src' to memory area 'dest'.
 *
 * @param:   void*          dest,
 * @param:   const void*    src,
 * @param:   size_t         n
 *
 * @return:  void*          dest
**/
__forceinline
void* vpr_deviate_memcpy( void*         dest,
                          const void*   src,
                          size_t        n_bytes )
{
    unsigned char* d = (unsigned char *)dest;
    const unsigned char* s = (const unsigned char *)src;
    for (size_t i = 0; i < n_bytes; ++i) {
        d[i] = s[i];
    }

    return dest;
}

/**
 * Resolve dynamic address using address and pointer array.
 *
 * @param:  uintptr_t       address
 * @param:  const uint16_t* offsets
 * @param:  const size_t    n_memb
 *
 * @return: uintptr_t       resolved_address
**/
__forceinline
uintptr_t vpr_deviate_resolve_dynamic_address( uintptr_t       address,
                                               const uint16_t* offsets,
                                               const size_t    n_memb )
{ 
    for (size_t i = 0; i < n_memb; i++) {
        if (!address) {
            return 0;
        }

        address = *(uintptr_t *)address;
        address += offsets[i];

        if (*(uintptr_t *)address == 0) {
            return 0;
        }
    }

    return address;
}

/**
 * Replace executable of destination from the source.
 *
 * @param:  void*           destination,
 * @param:  const void*     source,
 * @param:  const size_t    size
 *
 * @return: bool            success
**/
__forceinline
bool vpr_deviate_patch( void*        destination,
                        const void*  source,
                        const size_t size )
{
    PVOID base_address = destination;
    SIZE_T size_ = size;
    DWORD protect = 0;

    if (fNtProtectVirtualMemory((void *)-1, &base_address, &size_, PAGE_EXECUTE_READWRITE, &protect)) {
        return false;
    }

    vpr_deviate_memcpy(destination, source, size);

    if (fNtProtectVirtualMemory((void *)-1, &base_address, &size_, protect, &protect)) {
        return false;
    }

    return true;
}

/**
 * Detours the target to another function.
 *
 * @param:  void*            target_func,
 * @param:  const void*      detour_func,
 * @param:  void*            original_bytes,
 * @param:  const size_t     original_bytes_size
 *
 * @return: uint64_t         success
**/
__forceinline
uint64_t vpr_deviate_detour( void*        target_func,
                             const void*  detour_func,
                             void*        original_bytes,
                             const size_t original_bytes_size )
{
    if (original_bytes) {
        vpr_deviate_memcpy(original_bytes, target_func, original_bytes_size);
    }

    PVOID base_address = target_func;
    DWORD protect = 0;
    SIZE_T size = 0;
    uint64_t relative_func = (uintptr_t)detour_func - (uintptr_t)target_func - _rel_jmp_size_;
    if ((relative_func > 0xFFFFFFFFllu)) {
        size = sizeof(rax_jmp_data_t);
        fNtProtectVirtualMemory((void *)-1, &base_address, &size, PAGE_EXECUTE_READWRITE, &protect);
        set_rax_jmp_data((rax_jmp_data_ptr)target_func, (uintptr_t)detour_func);
        fNtProtectVirtualMemory((void *)-1, &base_address, &size, protect, &protect);

        return sizeof(rax_jmp_data_t);
    }

    size = _rel_jmp_size_;
    fNtProtectVirtualMemory((void *)-1, &base_address, &size, PAGE_EXECUTE_READWRITE, &protect);
    set_rel_jmp_data((rel_jmp_data_ptr)target_func, (int32_t)relative_func);
    fNtProtectVirtualMemory((void *)-1, &base_address, &size, protect, &protect);

    return sizeof(rel_jmp_data_t);
}

/**
 * Detours the target to another function then returns a gateway address to the original target.
 *
 * @param:  void*           target_func,
 * @param:  const void*     detour_func,
 * @param:  size_t          detour_size,
 * @param:  void*           original_bytes,
 * @param:  size_t          original_bytes_size
 *
 * @return: void*           gateway_address (needs to be freed via VirtualFree by the caller when non-zero)
**/
__forceinline
void* vpr_deviate_trampoline( void*       target_func,
                              const void* detour_func,
                              void*       original_bytes,
                              size_t      original_bytes_size )
{
    if (original_bytes) {
        vpr_deviate_memcpy(original_bytes, target_func, original_bytes_size);
    }

    void* gateway = nullptr;
    if (!(gateway = VirtualAlloc(NULL, (2 * sizeof(rax_jmp_data_t)), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))) {
        return NULL;
    }
    vpr_deviate_memcpy(gateway, target_func, sizeof(rax_jmp_data_t));
    set_rax_jmp_data((rax_jmp_data_ptr)((uintptr_t)gateway+sizeof(rax_jmp_data_t)), (uint64_t)(target_func)+sizeof(rax_jmp_data_t));

    PVOID base_address = target_func;
    SIZE_T size_ = 2 * sizeof(rax_jmp_data_t);
    DWORD protect = 0;

    fNtProtectVirtualMemory((void *)-1, &base_address, &size_, PAGE_EXECUTE_READWRITE, &protect);
    set_rax_jmp_data((rax_jmp_data_ptr)(target_func), (uint64_t)detour_func);
    fNtProtectVirtualMemory((void *)-1, &base_address, &size_, protect, &protect);

    return gateway;
}


////////////////////////////////////////////////////////////////////////////////
//                                C++ API
////////////////////////////////////////////////////////////////////////////////


#if        defined(__cplusplus)
namespace vpr {
namespace deviate {

/**
 * Resolve dynamic address using address and pointer array.
 *
 * @param:  uintptr_t       address
 * @param:  const uint16_t* offsets
 * @param:  const size_t    n_memb
 *
 * @return: uintptr_t       resolved_address
**/
__forceinline
uintptr_t resolve_dynamic_address( uintptr_t       address,
                                   const uint16_t* offsets,
                                   const size_t    n_memb )
{ 
    return vpr_deviate_resolve_dynamic_address( address,
                                                offsets,
                                                n_memb );
}

/**
 * Replace executable of destination from the source.
 *
 * @param:  auto&&          destination
 * @param:  auto&&          source
 * @param:  const size_t    n_bytes
 *
 * @return: bool            success
**/
__forceinline
bool patch( auto&&       destination,
            auto&&       source,
            const size_t size )
{
    return vpr_deviate_patch( (void *)+destination,
                              (const void *)+source,
                              size );
}

/**
 * Detours the target to another function.
 *
 * @param:  auto&&          target_func
 * @param:  auto&&          detour_func
 * @param:  auto&&          original_bytes
 * @param:  const size_t    original_bytes_size
 *
 * @return: uint64_t         success
**/
__forceinline
uint64_t detour( auto&&       target_func,
                 auto&&       detour_func,
                 auto&&       original_bytes = nullptr,
                 const size_t original_bytes_size = _rel_jmp_size_ )
{
    return vpr_deviate_detour( (void *)+target_func,
                               (const void *)+detour_func,
                               original_bytes,
                               original_bytes_size );
}

/**
 * Detours the target to another function then returns a gateway address to the original target.
 *
 * @param:  auto&&          target_func
 * @param:  auto&&          detour_func
 * @param:  size_t          detour_size
 * @param:  auto&&          original_bytes
 * @param:  size_t          original_bytes_size
 *
 * @return: void*           gateway_address (needs to be freed via VirtualFree by the caller when non-zero)
**/
__forceinline
void* trampoline( auto&&       target_func,
                  auto&&       detour_func,
                  auto&&       original_bytes = nullptr,
                  const size_t original_bytes_size = _rel_jmp_size_ )
{
    return vpr_deviate_trampoline( (void *)+target_func,
                                   (const void *)+detour_func,
                                   original_bytes,
                                   original_bytes_size );

}

////////////////////////////////////////////////////////////////////////////////
//                               Hook Manager
////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] interceptor {
    enum hook_type : int16_t {
        unhooked = -1,
        detour_t = 0,
        tramp_t  = 1
    };
public:
    interceptor() = delete;
    interceptor(const interceptor &) = delete;
    interceptor& operator = (const interceptor &) = delete;
    interceptor(interceptor&&) = delete;
    interceptor& operator = (interceptor &&) = delete;

    __forceinline
    explicit constexpr interceptor( auto&& target_func,
                                    auto&& detour_func )
    : target_func_((uintptr_t)+target_func)
    , detour_func_((uintptr_t)+detour_func)
    , original_data_( *((original_data_ptr)target_func) )
    , hook_(hook_type::unhooked)
    {
    }

    __forceinline
    constexpr uint64_t relative_addr() const {
        return detour_func_ - target_func_ - _rel_jmp_size_;
    }

    __forceinline
    uint64_t detour() const {
        uint64_t address = vpr::deviate::detour( target_func_,
                                     detour_func_,
                                     nullptr );

        if (address) {
            hook_ = hook_type::detour_t;
        }

        return address;
    }

    __forceinline
    uint64_t detour( auto&&       original_bytes = nullptr,
                     const size_t original_bytes_size = _rel_jmp_size_ ) const
    {
        uint64_t address = vpr::deviate::detour( target_func_,
                                                 detour_func_,
                                                 original_bytes,
                                                 original_bytes_size );

        if (address) {
            hook_ = hook_type::detour_t;
        }

        return address;
    }

    __forceinline
    bool restore() const {
        if (hook_ == -1) {
            return false;
        }

        bool success = vpr::deviate::patch( target_func_,
                                            &original_data_,
                                            hook_ == hook_type::detour_t ?         // detour_t
                                                ( relative_addr() < 0x100000000 ?
                                                    sizeof(rel_jmp_data_t)    :
                                                    sizeof(rax_jmp_data_t) )
                                                : 2 * sizeof(rax_jmp_data_t) );    // tramp_t

        if (success) {
            hook_ = hook_type::unhooked;
            return true;
        }

        return false;
    }

    __forceinline
    void* trampoline() const {
        void* gateway =  vpr::deviate::trampoline( target_func_,
                                                   detour_func_,
                                                   nullptr,
                                                   2 * sizeof(rax_jmp_data_t) );

        if (gateway) {
            hook_ = hook_type::tramp_t;
        }
        
        return gateway;
    }

    __forceinline
    void* trampoline( auto&&       original_bytes = nullptr,
                      const size_t original_bytes_size = _rel_jmp_size_ ) const
    {
        void* gateway = vpr::deviate::trampoline( target_func_,
                                                  detour_func_,
                                                  original_bytes,
                                                  original_bytes_size );

        if (gateway) {
            hook_ = hook_type::tramp_t;
        }
        
        return gateway;
    }

private:
    typedef struct {
        unsigned char bytes[64];
    } original_data, *original_data_ptr;

    uintptr_t           target_func_;
    uintptr_t           detour_func_;
    const original_data original_data_;
    mutable hook_type   hook_;
}; // class interceptor

} // namespace memory
} // namespace vpr
#endif  // defined(__cplusplus)


#endif  // VPR_DEVIATE_HEADER
