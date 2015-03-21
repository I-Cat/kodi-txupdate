/*
 *      Copyright (C) 2005-2014 Team Kodi
 *      http://xbmc.org
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

#ifndef LANGCODES_H
#define LANGCODES_H

#pragma once

#include <string>
#include <map>
#include <list>

struct CLangcodes
{
  std::string Gui_langname;
  std::string Old_langcode;
  std::string Tx_langname;
  std::string New_langcode;
  int nplurals;
  std::string Pluralform;
};

class CLCodeHandler
{
public:
  CLCodeHandler();
  ~CLCodeHandler();
  void Init(std::string strURL);
  std::string FindLangCode(std::string Lang);
  std::string FindLang(std::string LangCode);
  int GetnPlurals(std::string LangToLook);
  std::string GetPlurForm(std::string LangToLook);
  std::string VerifyLangCode(std::string LangCode);
private:
  std::map <std::string, CLangcodes> m_mapLCodes;
  std::map <std::string, CLangcodes>::iterator itmapLCodes;
};

extern CLCodeHandler g_LCodeHandler;

#endif
