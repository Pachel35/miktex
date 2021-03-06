/* appnames.cpp: managing application names

   Copyright (C) 1996-2016 Christian Schenk

   This file is part of the MiKTeX Core Library.

   The MiKTeX Core Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   The MiKTeX Core Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the MiKTeX Core Library; if not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. */

#include "StdAfx.h"

#include "internal.h"

#include "miktex/Core/CsvList.h"

#include "Session/SessionImpl.h"

using namespace MiKTeX::Core;
using namespace std;

inline void AppendTag(string & str, const string & tag)
{
  if (!str.empty())
  {
    str += PATH_DELIMITER;
  }
  str += tag;
}

void SessionImpl::PushAppName(const string & name)
{
  MIKTEX_ASSERT(name.find(PATH_DELIMITER) == string::npos);
  string newApplicationNames = name;
  for (CsvList tag(applicationNames, PATH_DELIMITER); tag; ++tag)
  {
    // stop at the miktex application tag; this is always the last tag
    if (Utils::EqualsIgnoreCase(*tag, "miktex"))
    {
#if defined(MIKTEX_DEBUG)
      ++tag;
      MIKTEX_ASSERT(!tag);
#endif
      break;
    }
    if (Utils::EqualsIgnoreCase(*tag, name))
    {
      continue;
    }
    AppendTag(newApplicationNames, *tag);
  }
  AppendTag(newApplicationNames, "miktex");
  if (Utils::EqualsIgnoreCase(newApplicationNames, applicationNames))
  {
    return;
  }
  fileTypes.clear();
  applicationNames = newApplicationNames;
  trace_config->WriteLine("core", T_("application tags: ") + applicationNames);
}

void SessionImpl::PushBackAppName(const string & name)
{
  MIKTEX_ASSERT(name.find(PATH_DELIMITER) == string::npos);
  fileTypes.clear();
  string newApplicationNames;
  for (CsvList tag(applicationNames, PATH_DELIMITER); tag; ++tag)
  {
    // stop at the miktex application tag; this is always the last tag
    if (Utils::EqualsIgnoreCase(*tag, "miktex"))
    {
#if defined(MIKTEX_DEBUG)
      ++tag;
      MIKTEX_ASSERT(!tag);
#endif
      break;
    }
    if (Utils::EqualsIgnoreCase(*tag, name))
    {
      continue;
    }
    AppendTag(newApplicationNames, *tag);
  }
  AppendTag(newApplicationNames, name);
  AppendTag(newApplicationNames, "miktex");
  if (Utils::EqualsIgnoreCase(newApplicationNames, applicationNames))
  {
    return;
  }
  applicationNames = newApplicationNames;
  trace_config->WriteLine("core", T_("application tags: ") + applicationNames);
}
