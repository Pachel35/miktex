/* Copyright (C) 2000-2012 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef _WIN32

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#ifdef _MSC_VER
#include <io.h>
#endif

FILE* tmpfile2() {
	FILE *fp = 0;
	char fname[MAX_PATH + 1];
	if (GetTempFileName(".", "tmp", 0, fname)) {
		int fd = _open(fname, _O_CREAT | _O_TEMPORARY | _O_RDWR | _O_BINARY, _S_IREAD | _S_IWRITE);
		if (fd >= 0) {
			fp = _fdopen(fd, "w+b");
			if (fp == NULL) {
				int saved_errno = errno;
				_close(fd);
				errno = saved_errno;
			}
		}
	}
	return fp;
}

#endif

#if 0

struct library_version_configuration exe_library_version_configuration = {
    FONTFORGE_LIBFFE_VERSION_MAJOR,
    FONTFORGE_LIBFFE_VERSION_MINOR,
    LibFF_ModTime,			/* Seconds since 1970 (standard unix time) */
    LibFF_ModTime_Str,			/* Version date (in char string format)    */
    LibFF_VersionDate,			/* Version as long value, Year, month, day */
    sizeof(struct library_version_configuration),
    sizeof(struct splinefont),
    sizeof(struct splinechar),
    sizeof(struct fontviewbase),
    sizeof(struct charviewbase),
    sizeof(struct cvcontainer),
    1,
    1,

#ifdef _NO_PYTHON
    0,
#else
    1,
#endif
    0xff		/* Not currently defined */
};
#endif // 0

