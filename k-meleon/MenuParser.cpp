/*
*  Copyright (C) 2000 Brian Harris
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "StdAfx.h"
#include <afxtempl.h>

#include "MfcEmbed.h"
extern CMfcEmbedApp theApp;

#include "Plugins.h"
#include "kmeleon_plugin.h"

#include "MenuParser.h"
#include "Utils.h"

CMenuParser::CMenuParser(){
}

CMenuParser::CMenuParser(CString &filename){
	Load(filename);
}

CMenuParser::~CMenuParser(){
  POSITION pos = menus.GetStartPosition();
  CMenu *m;
  CString s;
  while (pos){
    menus.GetNextAssoc( pos, s, m);
    if (m){
      m->DestroyMenu();
      delete m;
    }
  }
}


BOOL CALLBACK MenuLogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
   if (uMsg == WM_INITDIALOG) {
      CString *log = (CString *)lParam;
      SetDlgItemText(hwndDlg, IDC_ERRORS, *log);
      SetWindowText(hwndDlg, _T("Menu Log"));
      return true;
   }
   else if (uMsg == WM_COMMAND) {
      if (LOWORD(wParam) == IDOK)
         EndDialog(hwndDlg, 0);
    return true;
  }
  return false;
}

class CMenuLog {
protected:
   CString log;
   int error;
public:
   CMenuLog() {
      error = 0;
   };
   ~CMenuLog() {
      if (error)
         DialogBoxParam(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_ERRORBOX), NULL, MenuLogProc, (LPARAM)&log);
   };

   void WriteString(const char *string){
      log += string;
   }

   void Error(){
      error = 1;
   }
};

#if 0
#define LOG_1(msg, var1)       \
   if (log) {                  \
      char err[255];            \
      sprintf(err, msg, var1);  \
      log->WriteString(err);    \
      log->WriteString("\r\n"); \
   }

#define LOG_2(msg, var1, var2)      \
   if (log) {                       \
      char err[255];                 \
      sprintf(err, msg, var1, var2); \
      log->WriteString(err);         \
      log->WriteString("\r\n");      \
   }
#else
#define LOG_1(a, b)
#define LOG_2(a, b, c)
#endif

#define LOG_ERROR_1(msg, var1)   \
   if (log) {                     \
      log->Error();               \
      char err[255];              \
      sprintf(err, msg, var1);    \
      log->WriteString("*** ");    \
      log->WriteString(err);      \
      log->WriteString("\r\n");   \
   }

#define LOG_ERROR_2(msg, var1, var2)   \
   if (log) {                     \
      log->Error();               \
      char err[255];              \
      sprintf(err, msg, var1, var2);    \
      log->WriteString("*** ");    \
      log->WriteString(err);      \
      log->WriteString("\r\n");   \
   }

int CMenuParser::Load(CString &filename){
   CFile *menuFile;
   TRY {
      menuFile = new CFile(filename, CFile::modeRead);
   }
   CATCH (CFileException, e) {
      menuFile = NULL;
      return 0;
   }
   END_CATCH

   CMenuLog *log;
   TRY {
      log = new CMenuLog(); //("menu.log", CFile::modeWrite | CFile::modeCreate);
      log->WriteString("Menu File: ");
      log->WriteString(filename);
      log->WriteString("\r\n\r\n");
   }
   CATCH (CFileException, e) {
      log = NULL;
   }
   END_CATCH

  long length = menuFile->GetLength();
  char *buffer = new char[length + 1];
  menuFile->Read(buffer, length);
  // force the terminating 0
  buffer[length] = 0;
  
  menuFile->Close();
  delete menuFile;
  menuFile = NULL;

  TranslateTabs(buffer);

  CMap<CString, LPCSTR, int, int &> defineMap;
#include "defineMap.cpp"

  CMenu *currentMenu = NULL;

  char *p = strtok(buffer, "\r\n");
  while (p){
    if (p[0] == '#'){
    }
    else if (!currentMenu){
      // There can only be 2 things outside a menu:
      //   comments, and the beginning of a menu block
      char *cb = strchr(p, '{');
      if (cb){
        *cb = 0;

        p = SkipWhiteSpace(p);
        TrimWhiteSpace(p);

        currentMenu = new CMenu();

        if (strstr(p, _T("Popup"))){
          currentMenu->CreatePopupMenu();
        }else{
          currentMenu->CreateMenu();
        }
        CMenu *popup = NULL;
        if (menus.Lookup(CString(p), popup)){
          popup->DestroyMenu();
          delete popup;
        }
        menus[p] = currentMenu;

        LOG_1("Created Menu %s", p);
      }
    }
    else{
      p = SkipWhiteSpace(p);
      TrimWhiteSpace(p);

      if (p[0] == ':'){
        p ++;
        CMenu *popup = NULL;
        menus.Lookup(CString(p), popup);
        if (popup){
          currentMenu->AppendMenu(MF_POPUP | MF_STRING, (UINT)popup->m_hMenu, p);
          LOG_1("Added popup %s", p);
        }else{
          LOG_ERROR_1("Popup %s not found!", p);
        }
      }
      else if (p[0] == '-'){
        currentMenu->AppendMenu(MF_SEPARATOR);
        LOG_1("Added Seperator", 0);
      }
      else if (p[0] == '}'){
        currentMenu = NULL;
        LOG_1("Ended Menu", 0);
      }
      else {
        // it's either a plugin or a menu item
        char *op = strchr(p, '(');
        if (op){ // if there's an open parenthesis, we'll assume it's a plugin
          char *parameter = op + 1;
          char *cp = strrchr(parameter, ')');
          if (cp) *cp = 0;
          *op = 0;

          TrimWhiteSpace(p);

          kmeleonPlugin * kPlugin = theApp.plugins.Load(p);

          if (kPlugin) {
            if (kPlugin->pf->DoMenu){
              kPlugin->pf->DoMenu(currentMenu->GetSafeHmenu(), parameter);

              LOG_2("Called plugin %s with parameter %s", p, parameter);
            }else{
              LOG_ERROR_1( "Plugin %s has no menu", p);
            }
          }else{
            LOG_ERROR_2( "Could not load plugin %s\r\n\twith parameter %s", p, parameter );
          }
        }
        else {
          char *e = strrchr(p, '=');
          if (e){
            *e = 0;
            e = SkipWhiteSpace(e+1);
            int val;
            if (!defineMap.Lookup(e, val)){
              val = atoi(e);
            }
            TrimWhiteSpace(p);
            currentMenu->AppendMenu(MF_STRING, val, p);

            LOG_2("Added menu item %s with command %d", p, val);
          }
          else {
            LOG_ERROR_1("I don't know what to do with %s", p);
          }
        }
      }
    } // currentMenu
    p = strtok(NULL, "\r\n");
  } // while

  delete [] buffer;

  if (log){
    delete log;
  }

  return 1;
}

CMenu *CMenuParser::GetMenu(char *menuName){
  CMenu *menu;
  if (!menus.Lookup(menuName, menu)){
    return NULL;
  }
	return menu;
}
