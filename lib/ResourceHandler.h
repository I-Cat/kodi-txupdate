/*
 *      Copyright (C) 2014 Team Kodi
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
#pragma once

#include "POHandler.h"
#include "AddonXMLHandler.h"
#include "UpdateXMLHandler.h"
#include <list>

typedef std::map<std::string, CPOHandler>::iterator T_itmapPOFiles;
typedef std::map <unsigned long long, CPOEntry>::iterator T_itPOData;

class CResourceHandler
{
public:
  CResourceHandler();
  CResourceHandler(const CXMLResdata& XMLResdata);
  ~CResourceHandler();
  bool FetchPOFilesTXToMem();
  bool FetchPOFilesUpstreamToMem();
  void MergeResource();

  void GenerateMergedPOFiles();
  void GenerateUpdatePOFiles();
  void WriteMergedPOFiles(const std::string& sAddonXMLPath, const std::string& sLangAddonXMLPath, const std::string& sChangeLogPath, const std::string& sLangPath);
  void WriteUpdatePOFiles(const std::string& strPath);

//  size_t GetLangsCount() const {return m_mapPOFiles.size();}
//  std::string GetLangCodeFromPos(size_t pos) {T_itmapPOFiles it = IterateToMapIndex (m_mapPOFiles.begin(), pos); return it->first;}
  CPOHandler* GetPOData(std::string strLang);
  bool FindUPSEntry(const CPOEntry &EntryToFind);
  bool FindSRCEntry(const CPOEntry &EntryToFind);

//  void AddPOData(CPOHandler& POHandler, std::string strLang) {m_mapPOFiles[strLang] = POHandler;}
  CAddonXMLHandler * GetXMLHandler () {return &m_AddonXMLHandler;}
  void SetXMLHandler (CAddonXMLHandler XMLHandler) {m_AddonXMLHandler = XMLHandler;}
  void SetChangedLangsFromUpstream (std::list<std::string> lChanged) {m_lChangedLangsFromUpstream = lChanged;}
  std::list<std::string> GetChangedLangsFromUpstream () {return m_lChangedLangsFromUpstream;}
  void SetChangedLangsInAddXMLFromUpstream (std::list<std::string> lChanged) {m_lChangedLangsInAddXMLFromUpstream = lChanged;}
  std::list<std::string> GetChangedLangsInAddXMLFromUpstream () {return m_lChangedLangsInAddXMLFromUpstream;}
  void SetIfIsLangAddon (bool bIsLangAddon) {m_bIsLangAddon = bIsLangAddon;}
  bool GetIfIsLangaddon () {return m_bIsLangAddon;}

protected:

  void CreateMissingDirs(std::string strResRootDir, int resType);
  T_itmapPOFiles IterateToMapIndex(T_itmapPOFiles it, size_t index);
  bool ComparePOFiles(CPOHandler& POHandler1, CPOHandler& POHandler2);
  std::list<std::string> ParseAvailLanguagesTX(std::string strJSON, const std::string &strURL);
  std::set<std::string> GetAvailLangsGITHUB();
  std::list<std::string> CreateMergedLangList();
  bool FindUPSEntry(const std::string sLCode, CPOEntry &EntryToFind);
  bool FindTRXEntry(const std::string sLCode, CPOEntry &EntryToFind);
  T_itPOData GetUPSItFoundEntry();
  T_itPOData GetTRXItFoundEntry();

  std::map<std::string, CPOHandler> m_mapUPS;
  std::map<std::string, CPOHandler> m_mapTRX;
  std::map<std::string, CPOHandler> m_mapUPD;
  std::map<std::string, CPOHandler> m_mapMRG;


  CAddonXMLHandler m_AddonXMLHandler;
  std::list<std::string> m_lChangedLangsFromUpstream;
  std::list<std::string> m_lChangedLangsInAddXMLFromUpstream;
  bool m_bIsLangAddon;
  CXMLResdata m_XMLResData;

  bool m_bLastUPSHandlerFound;
  bool m_bLastTRXHandlerFound;
  T_itmapPOFiles m_lastUPSIterator;
  T_itmapPOFiles m_lastTRXIterator;
  std::string m_lastUPSLCode;
  std::string m_lastTRXLCode;
};
