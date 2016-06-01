///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun 17 2015)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __LMS7SUITEAPP_GUI_H__
#define __LMS7SUITEAPP_GUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/statusbr.h>
#include <wx/sizer.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class AppFrame_view
///////////////////////////////////////////////////////////////////////////////
class AppFrame_view : public wxFrame 
{
	private:
	
	protected:
		enum
		{
			idMenuQuit = 1000
		};
		
		wxMenuBar* mbar;
		wxMenu* fileMenu;
		wxMenu* mnuOptions;
		wxMenu* mnuModules;
		wxStatusBar* statusBar;
		wxFlexGridSizer* contentSizer;
		
		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnQuit( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowConnectionSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowDeviceInfo( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowSPI( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		AppFrame_view( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("Sony PA controls"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		
		~AppFrame_view();
	
};

#endif //__LMS7SUITEAPP_GUI_H__
