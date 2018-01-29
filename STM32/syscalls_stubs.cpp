#include <cerrno>
#include <sys/stat.h>

extern "C"
{

int __attribute__ ((weak)) _close(int file)
{
	errno = ENOTSUP;
	return -1;
}

int __attribute__ ((weak)) _fstat(int file, struct stat *st)
{
	st ->st_mode = S_IFCHR;
	return 0;
}


int __attribute__ ((weak)) _isatty(int file) {
	return 1;
}

int __attribute__ ((weak)) _link(char *old , char *n)
{
	errno = EMLINK;
	return -1;
}

int __attribute__ ((weak)) _lseek(int file , int ptr , int dir)
{
	errno = ENOTSUP;
	return -1;
}

int __attribute__ ((weak)) _open(const char *name , int flags , int mode)
{
	errno = ENOTSUP;
	return -1;
}


int __attribute__ ((weak)) _read(int file , char *ptr , int len)
{
	return 0;
}


int __attribute__ ((weak)) _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int __attribute__ ((weak)) _write(int file , char *ptr , int len)
{
	return 0;
}


}
