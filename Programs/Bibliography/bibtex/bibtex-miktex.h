/* bibtex-miktex.h:                                     -*- C++ -*-

   Copyright (C) 1996-2016 Christian Schenk

   This file is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 2, or (at your
   option) any later version.

   This file is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#if defined(_MSC_VER)
#  pragma once
#endif

#include <miktex/TeXAndFriends/config.h>

#if !defined(THEDATA)
#  define THEDATA(x) C4P_VAR(x)
#endif

#define IMPLEMENT_TCX 1

#include "bibtexdefs.h"

#include <miktex/Core/FileType>
#include <miktex/Core/Registry>
#include <miktex/TeXAndFriends/WebAppInputLine>

using namespace MiKTeX::TeXAndFriends;
using namespace MiKTeX::Core;

#if !defined(MIKTEXHELP_BIBTEX)
#  include <miktex/Core/Help>
#endif

class BIBTEXCLASS :
  public WebAppInputLine
{
public:
  template<typename T> T * Reallocate(T * & p, size_t n)
  {
    size_t amount = n * sizeof(T);
    p = reinterpret_cast<T *>(realloc(p, amount));
    if (p == nullptr && amount > 0)
    {
      FatalError(MIKTEXTEXT("Virtual memory exhausted."));
    }
    return p;
  }
  
private:
  template<typename T> T * Allocate(T * &  p, size_t n)
  {
    p = nullptr;
    return Reallocate(p, n);
  }

private:
  template<typename T> T * Free(T * & p)
  {
    return Reallocate(p, 0);
  }

private:
  std::shared_ptr<MiKTeX::Core::Session> session;
  
public:
  void Init(const std::string & programInvocationName) override
  {
    WebAppInputLine::Init(programInvocationName);
    session = GetSession();
#if defined(IMPLEMENT_TCX)
    EnableFeature (Feature::TCX);
#endif
    THEDATA(mincrossrefs) = session->GetConfigValue(MIKTEX_REGKEY_BIBTEX, "min_crossrefs", 2).GetInt();
    THEDATA(maxbibfiles) = 20;
    THEDATA(maxentints) = 3000;
    THEDATA(maxentstrs) = 3000;
    THEDATA(maxfields) = 100000; //5000;
    THEDATA(poolsize) = 65000;
    THEDATA(wizfnspace) = 3000;
    Allocate(THEDATA(bibfile), THEDATA(maxbibfiles));
    Allocate(THEDATA(biblist), THEDATA(maxbibfiles));
    Allocate(THEDATA(entryints), THEDATA(maxentints));
    Allocate(THEDATA(entrystrs), THEDATA(maxentstrs));
    Allocate(THEDATA(fieldinfo), THEDATA(maxfields));
    Allocate(THEDATA(spreamble), THEDATA(maxbibfiles));
    Allocate(THEDATA(strpool), THEDATA(poolsize));
    Allocate(THEDATA(wizfunctions), THEDATA(wizfnspace));
  }
  
public:
  void Finalize() override
  {
    Free(THEDATA(bibfile));
    Free(THEDATA(biblist));
    Free(THEDATA(entryints));
    Free(THEDATA(entrystrs));
    Free(THEDATA(fieldinfo));
    Free(THEDATA(spreamble));
    Free(THEDATA(strpool));
    Free(THEDATA(wizfunctions));
    WebAppInputLine::Finalize();
  }

#define OPT_MIN_CROSSREFS 1000
#define OPT_QUIET 1001

public:
  void AddOptions() override
  {
    WebAppInputLine::AddOptions();
    AddOption(MIKTEXTEXT("min-crossrefs\0Include item after N cross-refs; default 2."), OPT_MIN_CROSSREFS,  POPT_ARG_STRING, "N");
    AddOption(MIKTEXTEXT("quiet\0Suppress all output (except errors)."), OPT_QUIET, POPT_ARG_NONE);
    AddOption("silent", "quiet");
    AddOption("terse", "quiet");
  }
  
public:
  MiKTeX::Core::FileType GetInputFileType() const override
  {
    return MiKTeX::Core::FileType::BIB;
  }

public:
  std::string MIKTEXTHISCALL GetUsage() const override
  {
    return MIKTEXTEXT("[OPTION...] AUXFILE");
  }

public:
  bool ProcessOption(int opt, const std::string & optArg) override
  {
    bool done = true;
    switch (opt)
      {
      case OPT_MIN_CROSSREFS:
        THEDATA(mincrossrefs) = std::stoi(optArg);
        break;
      case OPT_QUIET:
        SetQuietFlag(true);
        break;
      default:
        done = WebAppInputLine::ProcessOption(opt, optArg);
        break;
      }
    return done;
  }
  
public:
  std::string MIKTEXTHISCALL TheNameOfTheGame() const override
  {
    return "BibTeX";
  }

public:
  unsigned long MIKTEXTHISCALL GetHelpId() const override
  {
    return MIKTEXHELP_BIBTEX;
  }

public:
  template<class T> bool OpenBstFile(T & f) const
  {
    const char * lpszFileName = THEDATA(nameoffile);
    MIKTEX_ASSERT_STRING(lpszFileName);
    PathName bstFileName (lpszFileName);
    if (!bstFileName.HasExtension())
    {
      bstFileName.SetExtension(".bst");
    }
    PathName path;
    if (!session->FindFile(bstFileName.ToString(), FileType::BST, path))
    {
      return false;
    }
    FILE * pfile = session->OpenFile(path.GetData(), FileMode::Open, FileAccess::Read, true);
    f.Attach(pfile, true);
#ifdef PASCAL_TEXT_IO
    get(f);
#endif
    return true;
  }
};

extern BIBTEXCLASS BIBTEXAPP;
#define THEAPP BIBTEXAPP

template<class T> inline void miktexbibtexrealloc(const char * lpszVar, T * & p, size_t n)
{
  p = THEAPP.Reallocate(p, n + 1);
}

template<class T> inline bool miktexopenbstfile(T & f)
{
  return THEAPP.OpenBstFile(f);
}

inline bool miktexhasextension (const char * lpszFileName, const char * lpszExtension)
{
  return PathName(lpszFileName).HasExtension(lpszExtension);
}

#include <miktex/TeXAndFriends/WebAppInputLine.inl>
