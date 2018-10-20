/***************************************************************
 * Name:      ModelPoserMain.h
 * Purpose:   Defines Application Frame
 * Author:    Ammar Qammaz ()
 * Created:   2018-10-20
 * Copyright: Ammar Qammaz (http://ammar.gr)
 * License:
 **************************************************************/

#ifndef MODELPOSERMAIN_H
#define MODELPOSERMAIN_H

//(*Headers(ModelPoserFrame)
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
//*)

class ModelPoserFrame: public wxFrame
{
    public:

        ModelPoserFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~ModelPoserFrame();

    private:

        //(*Handlers(ModelPoserFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        //*)

        //(*Identifiers(ModelPoserFrame)
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(ModelPoserFrame)
        wxStatusBar* StatusBar1;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // MODELPOSERMAIN_H
    