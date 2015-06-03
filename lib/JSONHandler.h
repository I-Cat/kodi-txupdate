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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#pragma once

#include <string>
#include <stdio.h>
#include <jsoncpp/json/json.h>
#include <map>
#include <list>
#include "Langcodes.h"
#include "UpdateXMLHandler.h"

class CJSONHandler
{
public:
  CJSONHandler();
  ~CJSONHandler();
  std::list<std::string> ParseResources(std::string strJSON);
  std::list<std::string> ParseAvailLanguagesTX(std::string strJSON, const std::string &strURL, const std::string &strTXLangformat, const CXMLResdata& XMLResData);
  std::list<std::string> ParseAvailLanguagesGITHUB(std::string strJSON, std::string strURL, std::string strLangformat,
                                                   std::string strAddonXMLURL, bool bIsLangAddon);
  void ParseAddonXMLVersionGITHUB(const std::string &strJSON, const std::string &strURL, const std::string &strAddXMLFilename, const std::string &strChlogname);
  std::map<std::string, CLangcodes> ParseTransifexLanguageDatabase(std::string strJSON);
  std::string CreateJSONStrFromPOStr(std::string const &strPO);
  std::string CreateNewresJSONStrFromPOStr(std::string strTXResname, std::string const &strPO);
  void ParseUploadedStringsData(std::string const &strJSON, size_t &stradded, size_t &strupd);
  void ParseUploadedStrForNewRes(std::string const &strJSON, size_t &stradded);
  void AddGeneralRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLeft, std::string strRight);
  void AddCustomRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLangformat, const std::string &strLeft, const std::string &strRight);
  void ParseLangDatabaseVersion(const std::string &strJSON, const std::string &strURL);
};
extern CJSONHandler g_Json;
#endif