// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#ifndef __CSourceInfo_h__
#define __CSourceInfo_h__

/** \file
 *  Source file information. */

namespace Puma {


class CFileInfo;
class CT_Token;
class Token;
class Unit;


/** \class CSourceInfo CSourceInfo.h Puma/CSourceInfo.h
 *  Source file information for an entity. Stores the 
 *  file information and start token of the entity in
 *  the source file. */
class CSourceInfo {
  CFileInfo *_FileInfo; 
  CT_Token *_StartToken;

public: 
  /** Constructor. */
  CSourceInfo ();
  /** Destructor. */
  ~CSourceInfo ();
  
  /** Check if the source file locations are equal. 
   *  \param si The source file location to compare with. */
  bool operator ==(const CSourceInfo &si) const;
  /** Check if the source file locations are not equal. 
   *  \param si The source file location to compare with. */
  bool operator !=(const CSourceInfo &si) const;
  /** Check if this source file location is before the given location. 
   *  \param si The source file location to compare with. */
  bool operator <(const CSourceInfo &si) const;
  /** Check if this source file location is behind the given location. 
   *  \param si The source file location to compare with. */
  bool operator >(const CSourceInfo &si) const;
  /** Check if this source file location equals or is before the given location. 
   *  \param si The source file location to compare with. */
  bool operator <=(const CSourceInfo &si) const;
  /** Check if this source file location equals or is behind the given location. 
   *  \param si The source file location to compare with. */
  bool operator >=(const CSourceInfo &si) const;

  /** Check if the source file locations are equal. 
   *  \param token The token in the source file to compare with. */
  bool operator ==(const CT_Token *token) const;
  /** Check if the source file locations are not equal. 
   *  \param token The token in the source file to compare with. */
  bool operator !=(const CT_Token *token) const;
  /** Check if this source file location is before the given location. 
   *  \param token The token in the source file to compare with. */
  bool operator <(const CT_Token *token) const;
  /** Check if this source file location is behind the given location. 
   *  \param token The token in the source file to compare with. */
  bool operator >(const CT_Token *token) const;
  /** Check if this source file location equals or is before the given location. 
   *  \param token The token in the source file to compare with. */
  bool operator <=(const CT_Token *token) const;
  /** Check if this source file location equals or is behind the given location. 
   *  \param token The token in the source file to compare with. */
  bool operator >=(const CT_Token *token) const;

  /** Check if the source file locations are equal. 
   *  \param token The token in the source file to compare with. */
  bool operator ==(const Token *token) const;

  /** Get the source file information. */
  CFileInfo *FileInfo () const;
  /** Get the start token of the entity in the source file. */
  CT_Token *StartToken () const;
  /** Get name of the source file. */
  const char *FileName () const;
  /** Get the line number of the entity in the source file. */
  int Line () const;
  /** Get the column number of the entity in the source file. */
  int Column () const;
  /** Get the token unit of the source file. */
  Unit *SrcUnit () const;

  /** Set the source file information. 
   *  \param file The source file. */
  void FileInfo (CFileInfo *file);
  /** Set the start token of the entity in the source file. 
   *  \param token The start token. */
  void StartToken (CT_Token *token);
};

inline CSourceInfo::CSourceInfo () :
  _FileInfo ((CFileInfo*)0),
  _StartToken ((CT_Token*)0)
 {}
 
inline CSourceInfo::~CSourceInfo ()
 {}

inline bool CSourceInfo::operator !=(const CSourceInfo &info) const 
 { return ! (*this == info); }
inline bool CSourceInfo::operator <=(const CSourceInfo &info) const 
 { return ! (*this > info); }
inline bool CSourceInfo::operator >=(const CSourceInfo &info) const 
 { return ! (*this < info); }

inline bool CSourceInfo::operator !=(const CT_Token *token) const 
 { return ! (*this == token); }
inline bool CSourceInfo::operator <=(const CT_Token *token) const 
 { return ! (*this > token); }
inline bool CSourceInfo::operator >=(const CT_Token *token) const 
 { return ! (*this < token); }

inline CFileInfo *CSourceInfo::FileInfo () const
 { return _FileInfo; }
inline CT_Token *CSourceInfo::StartToken () const
 { return _StartToken; }

inline void CSourceInfo::FileInfo (CFileInfo *info)
 { _FileInfo = info; }
inline void CSourceInfo::StartToken (CT_Token *token)
 { _StartToken = token; }


} // namespace Puma

#endif /* __CSourceInfo_h__ */
