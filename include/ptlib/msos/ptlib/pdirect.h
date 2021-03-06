/*
 * pdirect.h
 *
 * File system directory class.
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
 * $Revision: 31371 $
 * $Author: rjongbloed $
 * $Date: 2014-02-06 02:34:49 +0000 (Thu, 06 Feb 2014) $
 */

///////////////////////////////////////////////////////////////////////////////
// PDirectory

  protected:
    HANDLE hFindFile;
    WIN32_FIND_DATA fileinfo;
    bool InternalEntryCheck();

  public:
    static PString CreateFullPath(const PString & path, PBoolean isDirectory);

// End Of File ///////////////////////////////////////////////////////////////
