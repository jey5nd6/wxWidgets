/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_menu.cpp
// Purpose:     XRC resource for menus and menubars
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include <wx/xml/xml.h>

#if wxUSE_XRC && wxUSE_MENUS

#include "wx/xrc/xh_menu.h"

#ifndef WX_PRECOMP
    #include "wx/frame.h"
    #include "wx/log.h"
    #include "wx/menu.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxMenuXmlHandler, wxXmlResourceHandler);

wxMenuXmlHandler::wxMenuXmlHandler() :
        wxXmlResourceHandler(), m_insideMenu(false)
{
    XRC_ADD_STYLE(wxMENU_TEAROFF);
}

wxObject *wxMenuXmlHandler::DoCreateResource()
{
    if (m_class == wxT("wxMenu"))
    {
        wxMenu *menu = m_instance ? wxStaticCast(m_instance, wxMenu)
                                  : new wxMenu(GetStyle());

        wxString title = GetText(wxT("label"));
        wxString help = GetText(wxT("help"));

        bool oldins = m_insideMenu;
        m_insideMenu = true;
        CreateChildren(menu, true/*only this handler*/);
        m_insideMenu = oldins;

#if wxUSE_MENUBAR
        wxMenuBar *p_bar = wxDynamicCast(m_parent, wxMenuBar);
        if (p_bar)
        {
            p_bar->Append(menu, title);
        }
        else
#endif // wxUSE_MENUBAR
        {
            wxMenu *p_menu = wxDynamicCast(m_parent, wxMenu);
            if (p_menu)
            {
                p_menu->Append(GetID(), title, menu, help);
                if (HasParam(wxT("enabled")))
                    p_menu->Enable(GetID(), GetBool(wxT("enabled")));
            }
        }

        return menu;
    }

    else
    {
        wxMenu *p_menu = wxDynamicCast(m_parent, wxMenu);

        if (m_class == wxT("separator"))
            p_menu->AppendSeparator();
        else if (m_class == wxT("break"))
            p_menu->Break();
        else /*wxMenuItem*/
        {
            int id = GetID();
            wxString label = GetText(wxT("label"));
#if wxUSE_ACCEL
            wxString accel = GetText(wxT("accel"), false);
            wxVector<wxString> extraAccels;
            if (HasParam(wxT("extra-accels")))
            {
                wxXmlNode* const extraAccelsNode = GetParamNode(wxT("extra-accels"));
                wxXmlNode* node = extraAccelsNode->GetChildren();
                while (node)
                {
                    if (node->GetName() == wxT("accel"))
                        extraAccels.push_back(node->GetChildren()->GetContent());
                    node = node->GetNext();
                }
            }
#endif // wxUSE_ACCEL

            wxItemKind kind = wxITEM_NORMAL;
            if (GetBool(wxT("radio")))
                kind = wxITEM_RADIO;
            if (GetBool(wxT("checkable")))
            {
                if ( kind != wxITEM_NORMAL )
                {
                    ReportParamError
                    (
                        "checkable",
                        "menu item can't have both <radio> and <checkable> properties"
                    );
                }

                kind = wxITEM_CHECK;
            }

            wxMenuItem *mitem = new wxMenuItem(p_menu, id, label,
                                               GetText(wxT("help")), kind);
#if wxUSE_ACCEL
            if (!extraAccels.empty())
            {
                const int entriesSize = extraAccels.size();
                for (int i = 0; i < entriesSize; ++i)
                {
                    wxAcceleratorEntry entry;
                    if (entry.FromString(extraAccels[i]))
                        mitem->AddExtraAccel(entry);
                    else
                        ReportParamError
                        (
                            "extra-accels",
                            wxString::Format("cannot create accel from '%s\'", extraAccels[i])
                        );
                }
            }

            if (!accel.empty())
            {
                wxAcceleratorEntry entry;
                if (entry.FromString(accel))
                    mitem->SetAccel(&entry);
                else
                    ReportParamError
                    (
                        "accel",
                        wxString::Format("cannot create accel from '%s'", accel)
                    );
            }
#endif // wxUSE_ACCEL

#if !defined(__WXMSW__) || wxUSE_OWNER_DRAWN
            if (HasParam(wxT("bitmap")))
            {
                // currently only wxMSW has support for using different checked
                // and unchecked bitmaps for menu items
#ifdef __WXMSW__
                if (HasParam(wxT("bitmap2")))
                    mitem->SetBitmaps(GetBitmap(wxT("bitmap2"), wxART_MENU),
                                      GetBitmap(wxT("bitmap"), wxART_MENU));
                else
#endif // __WXMSW__
                    mitem->SetBitmap(GetBitmap(wxT("bitmap"), wxART_MENU));
            }
#endif
            p_menu->Append(mitem);
            mitem->Enable(GetBool(wxT("enabled"), true));
            if (kind == wxITEM_CHECK)
                mitem->Check(GetBool(wxT("checked")));
        }
        return NULL;
    }
}



bool wxMenuXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxMenu")) ||
           (m_insideMenu &&
               (IsOfClass(node, wxT("wxMenuItem")) ||
                IsOfClass(node, wxT("break")) ||
                IsOfClass(node, wxT("separator")))
           );
}

#if wxUSE_MENUBAR

wxIMPLEMENT_DYNAMIC_CLASS(wxMenuBarXmlHandler, wxXmlResourceHandler);

wxMenuBarXmlHandler::wxMenuBarXmlHandler() : wxXmlResourceHandler()
{
    XRC_ADD_STYLE(wxMB_DOCKABLE);
}

wxObject *wxMenuBarXmlHandler::DoCreateResource()
{
    wxMenuBar *menubar = NULL;

    const int style = GetStyle();
    wxASSERT_MSG(!style || !m_instance,
                 "cannot use <style> with pre-created menubar");

    if ( m_instance )
        menubar = wxDynamicCast(m_instance, wxMenuBar);
    if ( !menubar )
        menubar = new wxMenuBar(style);

    CreateChildren(menubar);

    if (m_parentAsWindow)
    {
        wxFrame *parentFrame = wxDynamicCast(m_parent, wxFrame);
        if (parentFrame)
            parentFrame->SetMenuBar(menubar);
    }

    return menubar;
}



bool wxMenuBarXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxMenuBar"));
}

#endif // wxUSE_MENUBAR

#endif // wxUSE_XRC && wxUSE_MENUS
