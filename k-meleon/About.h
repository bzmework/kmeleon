/*
*  Copyright (C) 2001 Jeff Doozan
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

#include "version.h"
#include "resource.h"
#include "BrowserView.h"

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

protected:
   CString m_credits;
   CString m_version;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
   //{{AFX_MSG(CMfcEmbedApp)
   afx_msg void OnHome();
   afx_msg void OnForum();
   //}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
