/*
 * MStreams.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


////////////////////////////////////////////////////////////////////////////////
//
// MStreams
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStreams.h"

// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStreams)
#endif


////////////////////////////////////////////////////////////////////////////////


MStreams mout;
MStreams mlog;
MStreams mgui;
//mgui.DumpToGui(true);
MStreams __mimp;
MStreams __mdep;
MStreams __merr;


////////////////////////////////////////////////////////////////////////////////


MStreams::MStreams() : ostream(new MStreamBuffer())
{
  // Construct an instance of MStreams

  m_Streams = dynamic_cast<MStreamBuffer*>(rdbuf());
}


////////////////////////////////////////////////////////////////////////////////


MStreams& MStreams::printf(const char* Format, ...)
{
  // Warning we have here a 1024 byte string size limit

  if (Format == 0) return (*this);

  va_list args;
  va_start(args, Format);

  // Format the string similar to sprintf ...
  const int max = 1024;
  char Text[max];
  if (vsnprintf(Text, max, Format, args) < 0) {
    // ignore this error
  }
  // Save the string:
  for (int c = 0; c < max; c++) {
    put(Text[c]);
    if (Text[c] == '\n') flush();
    if (Text[c] == '\0') {
      flush();
      break;
    }
  }

  va_end(args);

  return (*this);
}


////////////////////////////////////////////////////////////////////////////////


ostream& endl(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->endl();
    return l;
  } else {
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& show(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->show();
    return l;
  } else {
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& fatal(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    //buf->sputc("This is unfortunately a fatal error!");
    buf->show();
    abort();
    return l;
  } else {
    abort();
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& info(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->show();
    return l;
  } else {
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& warn(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->show(MStreamBuffer::c_Warn);
    return l;
  } else {
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& error(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->show(MStreamBuffer::c_Error);
    return l;
  } else {
    return std::endl(l);
  }
}


////////////////////////////////////////////////////////////////////////////////


ostream& logonly(ostream& l)
{
  // Only do some nice formatting:

  if (MStreamBuffer* buf = dynamic_cast<MStreamBuffer*>(l.rdbuf())) {
    buf->logonly();
    return l;
  } else {
    return std::endl(l);
  }
}


// MStream.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
