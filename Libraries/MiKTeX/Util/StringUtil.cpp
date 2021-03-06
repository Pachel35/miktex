/* StringUtil.cpp:

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Util Library.

   The MiKTeX Util Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.
   
   The MiKTeX Util Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with the MiKTeX Util Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "internal.h"

size_t StringUtil::AppendString(char * dest, size_t destSize, const char * source)
{
  // TODO: MIKTEX_ASSERT_STRING(lpszBuf);
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  size_t length;

  for (length = 0; length < destSize && dest[length] != 0; ++length)
  {
    ;
  }

  if (length == destSize)
  {
    FATAL_ERROR();
  }

  length += CopyString(&dest[length], destSize - length, source);

  return length;
}

MIKTEXSTATICFUNC(void) CopyString2(char * lpszBuf, size_t bufSize, const char * lpszSource, size_t count)
{
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  *lpszBuf = 0;

  if (count == 0)
  {
    return;
  }

  char * lpsz = lpszBuf;

  while (bufSize > 0 && count > 0 && (*lpsz++ = *lpszSource++) != 0)
  {
    --bufSize;
    --count;
  }

  if (bufSize == 0)
  {
    FATAL_ERROR();
  }

  if (count == 0)
  {
    *lpsz = 0;
  }
}

template<typename CharType> size_t GenericCopyString(CharType * lpszBuf, size_t bufSize, const CharType * lpszSource)
{
  // TODO: MIKTEX_ASSERT_CHAR_BUFFER(lpszBuf, bufSize);
  // TODO: MIKTEX_ASSERT_STRING(lpszSource);

  size_t length = StrLen(lpszSource);

  if (length >= bufSize)
  {
    FATAL_ERROR();
  }

  memcpy(lpszBuf, lpszSource, sizeof(CharType) * (length + 1));

  return length;
}

size_t StringUtil::CopyString(char * dest, size_t destSize, const char * source)
{
  return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyString(wchar_t * dest, size_t destSize, const wchar_t * source)
{
  return GenericCopyString(dest, destSize, source);
}

size_t StringUtil::CopyString(char * dest, size_t destSize, const wchar_t * source)
{
  return CopyString(dest, destSize, WideCharToUTF8(source).c_str());
}

size_t StringUtil::CopyString(wchar_t * dest, size_t destSize, const char * source)
{
  return CopyString(dest, destSize, UTF8ToWideChar(source).c_str());
}

bool StringUtil::Contains(const char * list, const char * element, const char * delims, bool ignoreCase)
{
  for (Tokenizer tok(list, delims); tok; ++tok)
  {
    if (StringCompare((*tok).c_str(), element, ignoreCase) == 0)
    {
      return true;
    }
  }
  return false;
}

string StringUtil::FormatStringVA(const char * format, va_list arglist)
{
  CharBuffer<char> autoBuffer;
  int n;
#if defined(_MSC_VER)
  n = _vscprintf(format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  autoBuffer.Reserve(n + 1);
  n = vsprintf_s(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    FATAL_ERROR();
  }
#else
  n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
  if (n < 0)
  {
    FATAL_ERROR();
  }
  else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
  {
    autoBuffer.Reserve(n + 1);
    n = vsnprintf(autoBuffer.GetData(), autoBuffer.GetCapacity(), format, arglist);
    if (n < 0)
    {
      FATAL_ERROR();
    }
    else if (static_cast<size_t>(n) >= autoBuffer.GetCapacity())
    {
      FATAL_ERROR();
    }
  }
#endif
  return autoBuffer.GetData();
}

string StringUtil::FormatString(const char * format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  string str;
  try
  {
    str = FormatStringVA(format, arglist);
  }
  catch (...)
  {
    va_end(arglist);
    throw;
  }
  return str;
}

wstring StringUtil::UTF8ToWideChar(const char * utf8)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(utf8);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from UTF-8 byte sequence to wide character string did not succeed.");
  }
}

string StringUtil::WideCharToUTF8(const wchar_t * wideChars)
{
  try
  {
    wstring_convert<codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(wideChars);
  }
  catch (const range_error &)
  {
    throw UtilException("Conversion from wide character string to UTF-8 byte sequence did not succeed.");
  }
}
