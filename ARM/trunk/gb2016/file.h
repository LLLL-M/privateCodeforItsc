#ifndef __FILE_H_
#define __FILE_H_

#include <string>
#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
typedef int FileHandle;
typedef mode_t FileMode;
#define RDONLY	O_RDONLY
#define WRONLY	O_WRONLY
#define RDWR	O_RDWR
#define INVALID_FILE_HANDLE	(-1)
#define SEEK_BEGIN	SEEK_SET

#else	//__WIN32__
#include <windows.h>
typedef HANDLE FileHandle;
typedef DWORD FileMode;
#define RDONLY	GENERIC_READ
#define WRONLY	GENERIC_WRITE
#define RDWR	(GENERIC_READ | GENERIC_WRITE)
#define INVALID_FILE_HANDLE INVALID_HANDLE_VALUE
#define SEEK_BEGIN	FILE_BEGIN
#ifndef SEEK_END
#define SEEK_END	FILE_END
#define SEEK_CUR	FILE_CURRENT
#define SEEK_SET	FILE_BEGIN
#endif

#endif

namespace HikFile
{
	class File
	{
	private:
		FileHandle handle;
		FileHandle mapHandle;
	public:
		File()
		{ 
			handle = INVALID_FILE_HANDLE; 
#ifdef __WIN32__
			mapHandle = NULL;
#endif
		}
		File(const char *path)
		{
			File();
            Open(path);
		}
		~File()
		{
			Close();
		}
        std::wstring StringToWString(const std::string & str)
		{
			std::wstring wstr(str.length(), L'\0');
			std::copy(str.begin(), str.end(), wstr.begin());
			return wstr;
		}
		//只拷贝低字节至string中
		std::string WStringToString(const std::wstring & wstr)
		{
			std::string str(wstr.length(), '\0');
			std::copy(wstr.begin(), wstr.end(), str.begin());
			return str;
		}
        bool Open(const char *path, FileMode mode = RDWR)
		{
			if (handle != INVALID_FILE_HANDLE)
                Close();
#ifdef __linux__
			handle = open(path, mode | O_CREAT, 0666);
#else	//__WIN32__
            handle = CreateFile(StringToWString(path).c_str(), mode, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
			return (handle != INVALID_FILE_HANDLE);
		}
        bool Isopen()
        {
            return (INVALID_FILE_HANDLE != handle);
        }

        long Seek(long offset, off_t pos)
		{
			if (handle == INVALID_FILE_HANDLE)
				return -1;
#ifdef __linux__
			return lseek(handle, offset, pos);
#else	//__WIN32__
			return SetFilePointer(handle, offset, 0, pos);
#endif
		}
        int Read(void *buf, const size_t size)
		{
			if (handle == INVALID_FILE_HANDLE)
				return -1;
            if (size == 0)
                return 0;
#ifdef __linux__
			return read(handle, buf, size);
#else	//__WIN32__
            unsigned long readsize = -1;
			ReadFile(handle, buf, size, &readsize, NULL);
            return int(readsize);
#endif
		}
        int Read(std::string str)
        {
            return Read((void *)str.c_str(), str.length());
        }

        int Write(const void *buf, const size_t size)
		{
			if (handle == INVALID_FILE_HANDLE)
				return -1;
#ifdef __linux__
			return write(handle, buf, size);
#else	//__WIN32__
            unsigned long writesize = -1;
			WriteFile(handle, buf, size, &writesize, NULL);
            return int(writesize);
#endif
		}
		bool Fsync()
		{
			if (handle == INVALID_FILE_HANDLE)
				return false;
#ifdef __linux__
			return (0 == fdatasync(handle));
#else	//__WIN32__
			return FlushFileBuffers(handle);
#endif
		}
        size_t Size(size_t filesize = 0)
		{
			long offset, size;
			if (handle == INVALID_FILE_HANDLE)
				return 0;
			offset = Seek(0, SEEK_CUR);
			if (filesize > 0)
			{
				size = filesize;
				if (offset > filesize)
					offset = filesize;
#ifdef __linux__
				ftruncate(handle, filesize);
#else	//__WIN32__
				Seek(filesize, SEEK_BEGIN);
				SetEndOfFile(handle);
#endif
			}
			else
			{
				size = Seek(0, SEEK_END);
			}
			Seek(offset, SEEK_BEGIN);
			return (size == -1) ? 0 : (size_t)size;
		}
		void *Mmap(unsigned int mapsize = 0)
		{
#ifdef __linux__
			void *m = mmap(NULL, mapsize ? mapsize : Size(), PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
			return (m == MAP_FAILED || m == NULL) ? NULL : m;
#else	//__WIN32__
			if (mapHandle == NULL)
				mapHandle = CreateFileMapping(handle, NULL, PAGE_READWRITE, 0, mapsize, NULL);
			if (mapHandle == NULL)
				return NULL;
			return MapViewOfFile(mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#endif
		}
        bool Msync(void *addr, size_t size)
		{
			if (addr == NULL)
				return false;
#ifdef __linux__
			return (msync(addr, size, MS_SYNC) == 0);
#else	//__WIN32__
			return (mapHandle == NULL) ? false : FlushViewOfFile(addr, size);
#endif
		}
        bool Unmap(void *addr, __attribute__((unused)) unsigned int size = 0)
		{
			if (addr == NULL)
				return false;
#ifdef __linux__
			return (0 == munmap(addr, size));
#else	//__WIN32__
			return (mapHandle == NULL) ? false : UnmapViewOfFile(addr);
#endif
		}
		void Close()
		{
			if (handle == INVALID_FILE_HANDLE)
				return;
#ifdef __linux__
			close(handle);
#else	//__WIN32__
			if (mapHandle != NULL)
				CloseHandle(mapHandle);
			CloseHandle(handle);
			mapHandle = NULL;
#endif
			handle = INVALID_FILE_HANDLE;
		}
		File & operator <<(int n)
		{
			Write(&n, sizeof(n));
			return *this;
		}
		File & operator >>(int & n)
		{
			Read(&n, sizeof(n));
			return *this;
		}
        File & operator <<(const std::string & str)
		{
			Write(str.c_str(), str.size());
			return *this;
		}
        File & operator >>(std::string & str)
		{
            Read(str);
			return *this;
		}
	protected:
	};
}

#endif
