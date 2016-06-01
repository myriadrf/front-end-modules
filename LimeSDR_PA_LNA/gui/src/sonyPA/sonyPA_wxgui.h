#ifndef LMS7002M_SonyPA_wxgui_H
#define LMS7002M_SonyPA_wxgui_H

/**
@file SonyPA_wxgui.h
@author Lime Microsystems
*/

#include <vector>
#include <wx/panel.h>
class wxStaticText;
class wxFlexGridSizer;
class wxButton;
class wxStaticBoxSizer;
class wxChoice;
class wxComboBox;
class wxCheckBox;

#include <vector>
#include <map>

namespace lime{
class LMS64CProtocol;
class IConnection;
}

class SonyPA_wxgui : public wxPanel
{
public:
    SonyPA_wxgui(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long styles = 0);
    virtual void Initialize(lime::IConnection* serPort);
    virtual ~SonyPA_wxgui();

    void ExportCache(wxCommandEvent& event);
    void ImportCache(wxCommandEvent& event);
    void ClearCache(wxCommandEvent& event);
    void RemoveEntry(wxCommandEvent& event);
    void AddEntry(wxCommandEvent& event);
    void ChangeState(wxCommandEvent& event);

protected:
    struct ControlsState
    {
        struct channel{
            int dac;
            int extBranch[3];
            int throughPath[3];
            int ssc1[3];
            int ssc2[3];
        };

        std::string name;
        int paDriver;
        channel A;
        channel B;

        ControlsState& operator= (const ControlsState &src)
        {
            name = src.name;
            paDriver = src.paDriver;
            A.dac = src.A.dac;
            B.dac = src.B.dac;
            for(int i=0; i<3; ++i)
            {
                A.extBranch[i] = src.A.extBranch[i];
                A.throughPath[i] = src.A.throughPath[i];
                A.ssc1[i] = src.A.ssc1[i];
                A.ssc2[i] = src.A.ssc2[i];
            }
            for(int i=0; i<3; ++i)
            {
                B.extBranch[i] = src.B.extBranch[i];
                B.throughPath[i] = src.B.throughPath[i];
                B.ssc1[i] = src.B.ssc1[i];
                B.ssc2[i] = src.B.ssc2[i];
            }
			return *this;
        }
    };

    int initializeDatabase();
    ControlsState GetGUIState();
    void SetGUIState(const ControlsState& state);

    wxChoice* cmbPAdriver;
    std::vector<long> tunerIds;
    std::vector<wxCheckBox*> chkEB;
    std::vector<wxCheckBox*> chkTP;
    std::vector<wxChoice*> cmbSSC1;
    std::vector<wxChoice*> cmbSSC2;
    wxButton* btnUpdateAll;
    wxChoice* cmbDAC_A;
    wxChoice* cmbDAC_B;
    wxComboBox* cmbCacheEntry;
    wxButton* btnAddEntry;
    wxButton* btnRemoveEntry;
    wxButton* btnClearCache;
    wxButton* btnImportCache;
    wxButton* btnExportCache;

private:
    std::map<std::string, ControlsState> cache;
    void DownloadAll(wxCommandEvent& event);
    void UploadAll(wxCommandEvent& event);
    std::vector<long> chEBids;
    std::vector<long> chTPids;
    std::vector<long> cmbSSC1ids;
    std::vector<long> cmbSSC2ids;

protected:
    lime::LMS64CProtocol* m_serPort;

    DECLARE_EVENT_TABLE()
};

#endif
