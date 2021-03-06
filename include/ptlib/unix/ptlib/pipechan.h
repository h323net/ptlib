/*
 * pipechan.h
 *
 * Sub-process communicating via pipe I/O channel.
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
 * $Revision: 29378 $
 * $Author: rjongbloed $
 * $Date: 2013-03-29 00:28:35 +0000 (Fri, 29 Mar 2013) $
 */

  protected:
    int m_toChildPipe[2];
    int m_fromChildPipe[2];
    int m_stderrChildPipe[2];
    int m_childPID;
    int m_returnCode;

// End Of File ////////////////////////////////////////////////////////////////
