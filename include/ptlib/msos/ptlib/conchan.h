/*
 * conchan.h
 *
 * Console I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 31101 $
 * $Author: rjongbloed $
 * $Date: 2013-12-18 01:36:42 +0000 (Wed, 18 Dec 2013) $
 */

///////////////////////////////////////////////////////////////////////////////
// PConsoleChannel

  public:
    ~PConsoleChannel();

    virtual PBoolean Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual PBoolean Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

  protected:
    bool InternalSetConsoleMode(DWORD bit, bool on);
    PWin32Handle m_hConsole;
    DWORD m_lastMouseState;

// End Of File ///////////////////////////////////////////////////////////////
