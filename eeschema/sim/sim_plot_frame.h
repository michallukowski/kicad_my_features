/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * Copyright (C) 2017 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SIM_PLOT_FRAME__
#define __SIM_PLOT_FRAME__

/**
 * @file sim_plot_frame.h
 *
 * Subclass of SIM_PLOT_FRAME_BASE, which is generated by wxFormBuilder.
 */

#include "sim_plot_frame_base.h"
#include "sim_types.h"

#include <kiway_player.h>
#include <dialogs/dialog_sim_settings.h>

#include <wx/event.h>

#include <list>
#include <memory>
#include <map>

class SCH_EDIT_FRAME;
class SCH_COMPONENT;

class SPICE_SIMULATOR;
class NETLIST_EXPORTER_PSPICE_SIM;
class SIM_PLOT_PANEL;
class SIM_THREAD_REPORTER;
class TUNER_SLIDER;

///> Trace descriptor class
class TRACE_DESC
{
public:
    TRACE_DESC( const NETLIST_EXPORTER_PSPICE_SIM& aExporter, const wxString& aName,
            SIM_PLOT_TYPE aType, const wxString& aParam );

    ///> Modifies an existing TRACE_DESC simulation type
    TRACE_DESC( const NETLIST_EXPORTER_PSPICE_SIM& aExporter,
            const TRACE_DESC& aDescription, SIM_PLOT_TYPE aNewType )
        : TRACE_DESC( aExporter, aDescription.GetName(), aNewType, aDescription.GetParam() )
    {
    }

    const wxString& GetTitle() const
    {
        return m_title;
    }

    const wxString& GetName() const
    {
        return m_name;
    }

    const wxString& GetParam() const
    {
        return m_param;
    }

    SIM_PLOT_TYPE GetType() const
    {
        return m_type;
    }

private:
    // Three basic parameters
    ///> Name of the measured net/device
    wxString m_name;

    ///> Type of the signal
    SIM_PLOT_TYPE m_type;

    ///> Name of the signal parameter
    wxString m_param;

    // Generated data
    ///> Title displayed in the signal list/plot legend
    wxString m_title;
};

/** Implementing SIM_PLOT_FRAME_BASE */
class SIM_PLOT_FRAME : public SIM_PLOT_FRAME_BASE
{
public:
    /** Constructor */
    SIM_PLOT_FRAME( KIWAY* aKiway, wxWindow* aParent );
    ~SIM_PLOT_FRAME();

    void StartSimulation();
    void StopSimulation();
    bool IsSimulationRunning();

    /**
     * @brief Creates a new plot panel for a given simulation type and adds it to the main
     * notebook.
     * @param aSimType is requested simulation type.
     * @return The new plot panel.
     */
    SIM_PLOT_PANEL* NewPlotPanel( SIM_TYPE aSimType );

    /**
     * @brief Adds a voltage plot for a given net name.
     * @param aNetName is the net name for which a voltage plot should be created.
     */
    void AddVoltagePlot( const wxString& aNetName );

    /**
     * @brief Adds a current plot for a particular device.
     * @param aDeviceName is the device name (e.g. R1, C1).
     * @param aParam is the current type (e.g. I, Ic, Id).
     */
    void AddCurrentPlot( const wxString& aDeviceName, const wxString& aParam );

    /**
     * @brief Adds a tuner for a component.
     */
    void AddTuner( SCH_COMPONENT* aComponent );

    /**
     * @brief Removes an existing tuner.
     * @param aTuner is the tuner to be removed.
     * @param aErase decides whether the tuner should be also removed from the tuners list.
     * Otherwise it is removed only from the SIM_PLOT_FRAME pane.
     */
    void RemoveTuner( TUNER_SLIDER* aTuner, bool aErase = true );

    /**
     * @brief Returns the currently opened plot panel (or NULL if there is none).
     */
    SIM_PLOT_PANEL* CurrentPlot() const;

    /**
     * Returns the netlist exporter object used for simulations.
     */
    const NETLIST_EXPORTER_PSPICE_SIM* GetExporter() const;

private:
    void LoadSettings( wxConfigBase* aCfg ) override;
    void SaveSettings( wxConfigBase* aCfg ) override;

    /**
     * @brief Adds a new plot to the current panel.
     * @param aName is the device/net name.
     * @param aType describes the type of plot.
     * @param aParam is the parameter for the device/net (e.g. I, Id, V).
     */
    void addPlot( const wxString& aName, SIM_PLOT_TYPE aType, const wxString& aParam );

    /**
     * @brief Removes a plot with a specific title.
     * @param aPlotName is the full plot title (e.g. I(Net-C1-Pad1)).
     * @param aErase decides if plot should be removed from corresponding TRACE_MAP (see m_plots).
     */
    void removePlot( const wxString& aPlotName, bool aErase = true );

    /**
     * @brief Reloads the current schematic for the netlist exporter.
     */
    void updateNetlistExporter();

    /**
     * @brief Updates plot in a particular SIM_PLOT_PANEL. If the panel does not contain
     * the plot, it will be added.
     * @param aDescriptor contains the plot description.
     * @param aPanel is the panel that should receive the update.
     * @return True if a plot was successfully added/updated.
     */
    bool updatePlot( const TRACE_DESC& aDescriptor, SIM_PLOT_PANEL* aPanel );

    /**
     * @brief Updates the list of currently plotted signals.
     */
    void updateSignalList();

    /**
     * @brief Updates the cursor values list.
     */
    void updateCursors();

    /**
     * @brief Filters out tuners for components that do not exist anymore.
     * Decisions are based on the current NETLIST_EXPORTER data.
     */
    void updateTuners();

    /**
     * @brief Applies component values specified using tunder sliders to the current netlist.
     */
    void applyTuners();

    /**
     * @brief Loads plot settings from a file.
     * @param aPath is the file name.
     * @return True if successful.
     */
    bool loadWorkbook( const wxString& aPath );

    /**
     * @brief Saves plot settings to a file.
     * @param aPath is the file name.
     * @return True if successful.
     */
    bool saveWorkbook( const wxString& aPath );

    /**
     * @brief Returns X axis for a given simulation type.
     */
    SIM_PLOT_TYPE GetXAxisType( SIM_TYPE aType ) const;

    // Menu handlers
    void menuNewPlot( wxCommandEvent& aEvent ) override;
    void menuOpenWorkbook( wxCommandEvent& event ) override;
    void menuSaveWorkbook( wxCommandEvent& event ) override;

    void menuExit( wxCommandEvent& event ) override
    {
        Close();
    }

    void menuSaveImage( wxCommandEvent& event ) override;
    void menuSaveCsv( wxCommandEvent& event ) override;
    void menuZoomIn( wxCommandEvent& event ) override;
    void menuZoomOut( wxCommandEvent& event ) override;
    void menuZoomFit( wxCommandEvent& event ) override;
    void menuShowGrid( wxCommandEvent& event ) override;
    void menuShowGridUpdate( wxUpdateUIEvent& event ) override;
    void menuShowLegend( wxCommandEvent& event ) override;
    void menuShowLegendUpdate( wxUpdateUIEvent& event ) override;

    // Event handlers
    void onPlotChanged( wxAuiNotebookEvent& event ) override;
    void onPlotClose( wxAuiNotebookEvent& event ) override;

    void onSignalDblClick( wxMouseEvent& event ) override;
    void onSignalRClick( wxListEvent& event ) override;

    void onSimulate( wxCommandEvent& event );
    void onSettings( wxCommandEvent& event );
    void onAddSignal( wxCommandEvent& event );
    void onProbe( wxCommandEvent& event );
    void onTune( wxCommandEvent& event );
    void onShowNetlist( wxCommandEvent& event );

    void onClose( wxCloseEvent& aEvent );

    void onCursorUpdate( wxCommandEvent& aEvent );
    void onSimUpdate( wxCommandEvent& aEvent );
    void onSimReport( wxCommandEvent& aEvent );
    void onSimStarted( wxCommandEvent& aEvent );
    void onSimFinished( wxCommandEvent& aEvent );

    // adjust the sash dimension of splitter windows after reading
    // the config settings
    // must be called after the config settings are read, and once the
    // frame is initialized (end of the Ctor)
    void setSubWindowsSashSize();

    // Toolbar buttons
    wxToolBarToolBase* m_toolSimulate;
    wxToolBarToolBase* m_toolAddSignals;
    wxToolBarToolBase* m_toolProbe;
    wxToolBarToolBase* m_toolTune;
    wxToolBarToolBase* m_toolSettings;

    SCH_EDIT_FRAME* m_schematicFrame;
    std::unique_ptr<NETLIST_EXPORTER_PSPICE_SIM> m_exporter;
    std::shared_ptr<SPICE_SIMULATOR> m_simulator;
    SIM_THREAD_REPORTER* m_reporter;

    typedef std::map<wxString, TRACE_DESC> TRACE_MAP;

    struct PLOT_INFO
    {
        ///> Map of the traces displayed on the plot
        TRACE_MAP m_traces;

        ///> Spice directive used to execute the simulation
        wxString m_simCommand;
    };

    ///> Map of plot panels and associated data
    std::map<SIM_PLOT_PANEL*, PLOT_INFO> m_plots;

    ///> List of currently displayed tuners
    std::list<TUNER_SLIDER*> m_tuners;

    // Trick to preserve settings between runs:
    // the DIALOG_SIM_SETTINGS is not destroyed after closing the dialog.
    // Once created it will be not shown (shown only on request) during a session
    // and will be destroyed only when closing the simulator frame.
    DIALOG_SIM_SETTINGS* m_settingsDlg;

    // Right click context menu for signals in the listbox
    class SIGNAL_CONTEXT_MENU : public wxMenu
    {
        public:
            SIGNAL_CONTEXT_MENU( const wxString& aSignal, SIM_PLOT_FRAME* aPlotFrame );

        private:
            void onMenuEvent( wxMenuEvent& aEvent );

            const wxString& m_signal;
            SIM_PLOT_FRAME* m_plotFrame;

            enum SIGNAL_CONTEXT_MENU_EVENTS
            {
                HIDE_SIGNAL,
                SHOW_CURSOR,
                HIDE_CURSOR
            };
    };

    ///> Panel that was used as the most recent one for simulations
    SIM_PLOT_PANEL* m_lastSimPlot;

    ///> imagelists uset to add a small coloured icon to signal names
    ///> and cursors name, the same color as the corresponding signal traces
    wxImageList* m_signalsIconColorList;

    ///> A string to store the path of saved workbooks during a session
    static wxString m_savedWorkbooksPath;

    // Variables for temporary storage:
    int m_splitterLeftRightSashPosition;
    int m_splitterPlotAndConsoleSashPosition;
    int m_splitterSignalsSashPosition;
    int m_splitterTuneValuesSashPosition;
};

// Commands
wxDECLARE_EVENT( EVT_SIM_UPDATE, wxCommandEvent );
wxDECLARE_EVENT( EVT_SIM_REPORT, wxCommandEvent );

// Notifications
wxDECLARE_EVENT( EVT_SIM_STARTED, wxCommandEvent );
wxDECLARE_EVENT( EVT_SIM_FINISHED, wxCommandEvent );

#endif // __sim_plot_frame__
