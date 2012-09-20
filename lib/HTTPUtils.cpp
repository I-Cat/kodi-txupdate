/*
 *      Copyright (C) 2012 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "Log.h"
#include "HTTPUtils.h"
// #include <curl/types.h>
#include <curl/easy.h>
#include "FileUtils/FileUtils.h"
#include <cctype>
#include "Settings.h"
#include "POHandler.h"
#include "JSONHandler.h"

CHTTPHandler g_HTTPHandler;

using namespace std;

CHTTPHandler::CHTTPHandler()
{
  m_curlHandle = curl_easy_init();
};

CHTTPHandler::~CHTTPHandler()
{
  Cleanup();
};

std::string CHTTPHandler::GetURLToSTR(std::string strURL)
{
  std::string strBuffer;
  std::string strCacheFile = CacheFileNameFromURL(strURL);
  bool bIsTooLongUrl = strCacheFile == "cache_for_long_URL_download";
  strCacheFile = m_strCacheDir + "GET" + strCacheFile;

  if (!FileExist(strCacheFile) || GetFileAge(strCacheFile) > g_Settings.GetHTTPCacheExpire() * 60)
  {
    long result = curlURLToCache(strCacheFile, strURL);
    if (result < 200 || result >= 400)
      return "";
  }

  strBuffer = ReadFileToStr(strCacheFile);

  if (bIsTooLongUrl)
    DeleteFile(strCacheFile);
  return strBuffer;
};

long CHTTPHandler::curlURLToCache(std::string strCacheFile, std::string strURL)
{
  CURLcode curlResult;
  FILE *dloadfile;

  strURL = URLEncode(strURL);

  CLoginData LoginData = GetCredentials(strURL);

    if(m_curlHandle) 
    {
      dloadfile = fopen(strCacheFile.c_str(),"wb");
      curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, Write_CurlData_File);
      if (!LoginData.strLogin.empty())
      {
        curl_easy_setopt(m_curlHandle, CURLOPT_USERNAME, LoginData.strLogin.c_str());
        curl_easy_setopt(m_curlHandle, CURLOPT_PASSWORD, LoginData.strPassword.c_str());
      }
      curl_easy_setopt(m_curlHandle, CURLOPT_FAILONERROR, true);
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, dloadfile);
      curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);

      curlResult = curl_easy_perform(m_curlHandle);
      long http_code = 0;
      curl_easy_getinfo (m_curlHandle, CURLINFO_RESPONSE_CODE, &http_code);

      fseek (dloadfile, 0, SEEK_END);
      size_t filesize=ftell (dloadfile);
      fclose(dloadfile);

      if (curlResult == 0 && http_code >= 200 && http_code < 400)
        CLog::Log(logINFO, "HTTPHandler: curlURLToCache finished with success from URL %s to cachefile %s, read filesize: %ibytes",
                  strURL.c_str(), strCacheFile.c_str(), filesize);
      else
      {
        CLog::Log(logINFO, "HTTPHandler: curlURLToCache finished with error code: %i from URL %s to localdir %s",
                  http_code, strURL.c_str(), strCacheFile.c_str());
        DeleteFile(strCacheFile);
      }
      return http_code;
    }
    else
      CLog::Log(logERROR, "HTTPHandler: curlURLToCache failed because Curl was not initalized");
    return 0;
};

void CHTTPHandler::ReInit()
{
  if (!m_curlHandle)
    m_curlHandle = curl_easy_init();
  else
    CLog::Log(logWARNING, "HTTPHandler: Trying to reinitalize an already existing Curl handle");
};

void CHTTPHandler::Cleanup()
{
  if (m_curlHandle)
  {
    curl_easy_cleanup(m_curlHandle);
    m_curlHandle = NULL;
  }
};

size_t Write_CurlData_File(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
};

size_t Read_CurlData_File(char *bufptr, size_t size, size_t nitems, FILE *stream) 
{
  size_t read;
  read = fread(bufptr, size, nitems, stream);
  return read;
}


size_t Write_CurlData_String(char *data, size_t size, size_t nmemb, string *buffer)
{
  size_t written = 0;
  if(buffer != NULL)
  {
    buffer -> append(data, size * nmemb);
    written = size * nmemb;
  }
  return written;
}

typedef struct
{ 
  std::string * pPOString; 
  size_t pos; 
} Tputstrdata; 


size_t Read_CurlData_String(void * ptr, size_t size, size_t nmemb, void * stream)
{
  if (stream)
  {
    printf ("called\n");
    Tputstrdata * pPutStrData = (Tputstrdata*) stream; 

    size_t available = (pPutStrData->pPOString->size() - pPutStrData->pos);

    if (available > 0)
    {
      size_t written = std::min(size * nmemb, available);
      memcpy(ptr, &pPutStrData->pPOString->at(pPutStrData->pos), written); 
      pPutStrData->pos += written;
      return written; 
    }
  }

  return 0; 
}


void CHTTPHandler::SetCacheDir(std::string strCacheDir)
{
  if (!DirExists(strCacheDir))
    MakeDir(strCacheDir);
  m_strCacheDir = strCacheDir + DirSepChar;
  CLog::Log(logINFO, "HTTPHandler: Cache directory set to: %s", strCacheDir.c_str());
};

std::string CHTTPHandler::CacheFileNameFromURL(std::string strURL)
{
  if (strURL.size() > 200)
  {
    CLog::Log(logWARNING, "HTTPHandler: Can't make HTTP cache for too long URL: %s", strURL.c_str());
    return "cache_for_long_URL_download";
  }
  std::string strResult;
  std::string strCharsToKeep = "/.=?";
  std::string strReplaceChars = "_-=?";

  std::string hexChars = "01234567890abcdef"; 

  for (std::string::iterator it = strURL.begin(); it != strURL.end(); it++)
  {
    if (isalnum(*it))
      strResult += *it;
    else
    {
      if (size_t pos = strCharsToKeep.find(*it) != std::string::npos)
        strResult += strReplaceChars[pos];
      else
      {
        strResult += "%";
        strResult += hexChars[(*it >> 4) & 0x0F];
        strResult += hexChars[*it & 0x0F];
      }
    }
  }

  return strResult;
};

bool CHTTPHandler::LoadCredentials (std::string CredentialsFilename)
{
  TiXmlDocument xmlPasswords;

  if (!xmlPasswords.LoadFile(CredentialsFilename.c_str()))
  {
    CLog::Log(logINFO, "HTTPHandler: No \".passwords.xml\" file exists in project dir. No password protected web download will be available.");
    return false;
  }

  CLog::Log(logINFO, "HTTPHandler: Succesfuly found the .passwsords.xml file: %s", CredentialsFilename.c_str());

  TiXmlElement* pRootElement = xmlPasswords.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="websites")
  {
    CLog::Log(logWARNING, "HTTPHandler: No root element called \"websites\" in xml file.");
    return false;
  }

  CLoginData LoginData;

  const TiXmlElement *pChildElement = pRootElement->FirstChildElement("website");
  while (pChildElement && pChildElement->FirstChild())
  {
    std::string strWebSitePrefix = pChildElement->Attribute("prefix");
    if (pChildElement->FirstChild())
    {
      const TiXmlElement *pChildLOGINElement = pChildElement->FirstChildElement("login");
      if (pChildLOGINElement && pChildLOGINElement->FirstChild())
        LoginData.strLogin = pChildLOGINElement->FirstChild()->Value();
      const TiXmlElement *pChildPASSElement = pChildElement->FirstChildElement("password");
      if (pChildPASSElement && pChildPASSElement->FirstChild())
        LoginData.strPassword = pChildPASSElement->FirstChild()->Value();

      m_mapLoginData [strWebSitePrefix] = LoginData;
      CLog::Log(logINFO, "HTTPHandler: found login credentials for website prefix: %s", strWebSitePrefix.c_str());
    }
    pChildElement = pChildElement->NextSiblingElement("website");
  }

  return true;
};

CLoginData CHTTPHandler::GetCredentials (std::string strURL)
{
  CLoginData LoginData;
  for (itMapLoginData = m_mapLoginData.begin(); itMapLoginData != m_mapLoginData.end(); itMapLoginData++)
  {
    if (strURL.find(itMapLoginData->first) != std::string::npos)
    {
      LoginData = itMapLoginData->second;
      return LoginData;
    }
  }

  return LoginData;
};

std::string CHTTPHandler::URLEncode (std::string strURL)
{
  std::string strOut;
  for (std::string::iterator it = strURL.begin(); it != strURL.end(); it++)
  {
    if (*it == ' ')
      strOut += "%20";
    else
      strOut += *it;
  }
  return strOut;
}



bool CHTTPHandler::PutFileToURL(std::string strFilePath, std::string strURL)
{
  std::string strBuffer;
  std::string strCacheFile = CacheFileNameFromURL(strURL);
  bool bIsTooLongUrl = strCacheFile == "cache_for_long_URL_download";
  strCacheFile = m_strCacheDir + "PUT" + strCacheFile;

  if (!bIsTooLongUrl && FileExist(strCacheFile) && ComparePOFiles(strCacheFile, strFilePath))
  {
    CLog::Log(logINFO, "HTTPHandler::PutFileToURL: not necesarry to upload file as it has not changed from last upload. File: %s",
              strFilePath.c_str());
    return true;
  }

  CLog::Log(logINFO, "HTTPHandler::PutFileToURL: Uploading file to Transifex: %s", strFilePath.c_str());

  long result = curlFileToURL(strFilePath, strURL);
  if (result < 200 || result >= 400)
  {
    CLog::Log(logERROR, "HTTPHandler::PutFileToURL: File upload was unsuccessful, http errorcode: %i", result);
    return false;
  }

  CLog::Log(logINFO, "HTTPHandler::PutFileToURL: File upload was successful so creating a copy at the .httpcache directory");
  CopyFile(strFilePath, strCacheFile);

  return true;
};

long CHTTPHandler::curlFileToURL(std::string strFilePath, std::string strURL)
{
  CURLcode curlResult;

  strURL = URLEncode(strURL);

  std::string strPO = ReadFileToStr(strFilePath);

  CJSONHandler JSONHandler;
  std::string strPOJson = JSONHandler.CreateJSONStrFromPOStr(strPO);

  Tputstrdata PutStrData;
  PutStrData.pPOString = &strPOJson;
  PutStrData.pos = 0;

  CLoginData LoginData = GetCredentials(strURL);

  if(m_curlHandle) 
  {
    struct curl_slist *headers=NULL;
    headers = curl_slist_append( headers, "Content-Type: application/json");
    headers = curl_slist_append( headers, "charsets: utf-8");

    curl_easy_setopt(m_curlHandle, CURLOPT_READFUNCTION, Read_CurlData_String);
    curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(m_curlHandle, CURLOPT_PUT, 1L);
    if (!LoginData.strLogin.empty())
    {
      curl_easy_setopt(m_curlHandle, CURLOPT_USERNAME, LoginData.strLogin.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_PASSWORD, LoginData.strPassword.c_str());
    }
    curl_easy_setopt(m_curlHandle, CURLOPT_FAILONERROR, true);
    curl_easy_setopt(m_curlHandle, CURLOPT_READDATA, &PutStrData);
    curl_easy_setopt(m_curlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)strPOJson.size());
    curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(m_curlHandle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(m_curlHandle, CURLOPT_VERBOSE, 1L);

    curlResult = curl_easy_perform(m_curlHandle);

    long http_code = 0;
    curl_easy_getinfo (m_curlHandle, CURLINFO_RESPONSE_CODE, &http_code);

    if (curlResult == 0 && http_code >= 200 && http_code < 400)
      CLog::Log(logINFO, "HTTPHandler::curlFileToURL finished with success from File %s to URL %s",
                strFilePath.c_str(), strURL.c_str());
      else
      {
        CLog::Log(logINFO, "HTTPHandler::curlFileToURL finished with error code: %i from file %s to URL %s",
                  http_code, strFilePath.c_str(), strURL.c_str());
      }
      return http_code;
  }
  else
    CLog::Log(logERROR, "HTTPHandler::curlFileToURL failed because Curl was not initalized");
  return 700;
};

bool CHTTPHandler::ComparePOFiles(std::string strPOFilePath1, std::string strPOFilePath2) const
{
  CPOHandler POHandler1, POHandler2;
  POHandler1.ParsePOStrToMem(ReadFileToStr(strPOFilePath1), strPOFilePath1);
  POHandler2.ParsePOStrToMem(ReadFileToStr(strPOFilePath2), strPOFilePath2);

  if (POHandler1.GetNumEntriesCount() != POHandler2.GetNumEntriesCount())
    return false;
  if (POHandler1.GetClassEntriesCount() != POHandler2.GetClassEntriesCount())
    return false;

  for (size_t POEntryIdx = 0; POEntryIdx != POHandler1.GetNumEntriesCount(); POEntryIdx++)
  {
    const CPOEntry * POEntry1 = POHandler1.GetNumPOEntryByIdx(POEntryIdx);
    const CPOEntry * POEntry2 = POHandler2.GetNumPOEntryByID(POEntry1->numID);

    if (!(*POEntry1 == *POEntry2))
      return false;
  }

  for (size_t POEntryIdx = 0; POEntryIdx != POHandler1.GetClassEntriesCount(); POEntryIdx++)
  {
    const CPOEntry * POEntry1 = POHandler1.GetClassicPOEntryByIdx(POEntryIdx);
    CPOEntry POEntryToFind = *POEntry1;
    if (!POHandler2.LookforClassicEntry(POEntryToFind))
      return false;
  }
  return true;
}