
#pragma once

/*
farrtl.cpp

��������������� ��������� CRT �������
*/

#ifdef _DEBUG
#define MEMCHECK
#endif

#ifdef MEMCHECK
void* xf_malloc(size_t size, const char* Function, const char* File, int Line);
void* xf_realloc(void* block, size_t size, const char* Function, const char* File, int Line);
void* xf_realloc_nomove(void* block, size_t size, const char* Function, const char* File, int Line);

char* DuplicateString(const char* str, const char* Function, const char* File, int Line);
wchar_t* DuplicateString(const wchar_t* str, const char* Function, const char* File, int Line);

void* operator new(size_t size, const char* Function, const char* File, int Line);
void* operator new(size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void* operator new[](size_t size, const char* Function, const char* File, int Line);
void* operator new[](size_t size, const std::nothrow_t& nothrow_value, const char* Function, const char* File, int Line) noexcept;
void operator delete(void* block, const char* Function, const char* File, int Line);
void operator delete[](void* block, const char* Function, const char* File, int Line);

#define xf_malloc(size) xf_malloc(size, __FUNCTION__, __FILE__, __LINE__)
#define xf_realloc(block, size) xf_realloc(block, size, __FUNCTION__, __FILE__, __LINE__)
#define xf_realloc_nomove(block, size) xf_realloc_nomove(block, size, __FUNCTION__, __FILE__, __LINE__)
#define DuplicateString(str) DuplicateString(str, __FUNCTION__, __FILE__, __LINE__)
#define new new(__FUNCTION__, __FILE__, __LINE__)
#else
void* xf_malloc(size_t size);
void* xf_realloc_nomove(void* block, size_t size);
void* xf_realloc(void* block, size_t size);
char* DuplicateString(const char* string);
wchar_t* DuplicateString(const wchar_t* string);
#endif

void PrintMemory();

void  xf_free(void* block);
char* xstrncpy(char* dest, const char* src, size_t DestSize);
wchar_t* xwcsncpy(wchar_t* dest, const wchar_t* src, size_t DestSize);

#define ALIGNAS(value, alignment) ((value+(alignment-1))&~(alignment-1))
#define ALIGN(value) ALIGNAS(value, sizeof(void*))
