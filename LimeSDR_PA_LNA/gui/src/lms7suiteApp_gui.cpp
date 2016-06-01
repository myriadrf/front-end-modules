///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "lms7suiteApp_gui.h"

///////////////////////////////////////////////////////////////////////////

AppFrame_view::AppFrame_view( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 600,-1 ), wxDefaultSize );
	
	mbar = new wxMenuBar( 0 );
	fileMenu = new wxMenu();
	wxMenuItem* menuFileQuit;
	menuFileQuit = new wxMenuItem( fileMenu, idMenuQuit, wxString( wxT("&Quit") ) + wxT('\t') + wxT("Alt+F4"), wxT("Quit the application"), wxITEM_NORMAL );
	fileMenu->Append( menuFileQuit );
	
	mbar->Append( fileMenu, wxT("&File") ); 
	
	mnuOptions = new wxMenu();
	wxMenuItem* mnuConnectionSettings;
	mnuConnectionSettings = new wxMenuItem( mnuOptions, wxID_ANY, wxString( wxT("ConnectionSettings") ) , wxEmptyString, wxITEM_NORMAL );
	mnuOptions->Append( mnuConnectionSettings );
	
	mbar->Append( mnuOptions, wxT("Options") ); 
	
	mnuModules = new wxMenu();
	wxMenuItem* mnuDeviceInfo;
	mnuDeviceInfo = new wxMenuItem( mnuModules, wxID_ANY, wxString( wxT("Device Info") ) , wxEmptyString, wxITEM_NORMAL );
	mnuModules->Append( mnuDeviceInfo );
	
	wxMenuItem* mnuSPI;
	mnuSPI = new wxMenuItem( mnuModules, wxID_ANY, wxString( wxT("SPI") ) , wxEmptyString, wxITEM_NORMAL );
	mnuModules->Append( mnuSPI );
	
	mbar->Append( mnuModules, wxT("Modules") ); 
	
	this->SetMenuBar( mbar );
	
	statusBar = this->CreateStatusBar( 2, wxST_SIZEGRIP, wxID_ANY );
	contentSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	contentSizer->AddGrowableCol( 0 );
	contentSizer->AddGrowableRow( 0 );
	contentSizer->SetFlexibleDirection( wxBOTH );
	contentSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	
	this->SetSizer( contentSizer );
	this->Layout();
	contentSizer->Fit( this );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( AppFrame_view::OnClose ) );
	this->Connect( menuFileQuit->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnQuit ) );
	this->Connect( mnuConnectionSettings->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowConnectionSettings ) );
	this->Connect( mnuDeviceInfo->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowDeviceInfo ) );
	this->Connect( mnuSPI->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowSPI ) );
}

AppFrame_view::~AppFrame_view()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( AppFrame_view::OnClose ) );
	this->Disconnect( idMenuQuit, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnQuit ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowConnectionSettings ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowDeviceInfo ) );
	this->Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( AppFrame_view::OnShowSPI ) );
	
}
