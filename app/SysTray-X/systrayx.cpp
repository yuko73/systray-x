#include "systrayx.h"

/*
 *	Local includes
 */
#include "debugwidget.h"
#include "preferencesdialog.h"
#include "systrayxlink.h"
#include "systrayxicon.h"
#include "windowctrl.h"

/*
 *	Qt includes
 */
#include <QCoreApplication>
#include <QMenu>
#include <QStyle>
#include <QIcon>

/*
 *  Constants
 */
const QString   SysTrayX::JSON_PREF_REQUEST = "{\"preferences\":{}}";


/*
 *  Constructor
 */
SysTrayX::SysTrayX( QObject *parent ) : QObject( parent )
{
    /*
     *  Setup preferences storage
     */
    m_preferences = new Preferences();

    /*
     *  Setup window control
     */
    m_win_ctrl = new WindowCtrl( m_preferences );

    /*
     *  Setup the link
     */
    m_link = new SysTrayXLink( m_preferences );

    /*
     *  Setup preferences dialog
     */
    m_pref_dialog = new PreferencesDialog( m_link, m_preferences );

    /*
     *  Setup tray icon
     */
    createTrayIcon();
    m_tray_icon->show();

    /*
     *  Setup debug window
     */
    m_debug = new DebugWidget( m_preferences );
    if( m_preferences->getDebug() ) {
        m_debug->show();
    }

    /*
     *  Connect debug link signals
     */
    connect( m_link, &SysTrayXLink::signalUnreadMail, m_debug, &DebugWidget::slotUnreadMail );

    connect( m_win_ctrl, &WindowCtrl::signalConsole, m_debug, &DebugWidget::slotConsole );
    connect( m_debug, &DebugWidget::signalTest1ButtonClicked, m_win_ctrl, &WindowCtrl::slotWindowTest1 );
    connect( m_debug, &DebugWidget::signalTest2ButtonClicked, m_win_ctrl, &WindowCtrl::slotWindowTest2 );
    connect( m_debug, &DebugWidget::signalTest3ButtonClicked, m_win_ctrl, &WindowCtrl::slotWindowTest3 );

    /*
     *  Connect preferences signals
     */
    connect( m_preferences, &Preferences::signalIconTypeChange, m_tray_icon, &SysTrayXIcon::slotIconTypeChange );
    connect( m_preferences, &Preferences::signalIconDataChange, m_tray_icon, &SysTrayXIcon::slotIconDataChange );

    connect( m_preferences, &Preferences::signalHideOnMinimizeChange, m_win_ctrl, &WindowCtrl::slotHideOnMinimizeChange );
    connect( m_preferences, &Preferences::signalStartMinimizedChange, m_win_ctrl, &WindowCtrl::slotStartMinimizedChange );


    connect( m_preferences, &Preferences::signalIconTypeChange, m_pref_dialog, &PreferencesDialog::slotIconTypeChange );
    connect( m_preferences, &Preferences::signalIconDataChange, m_pref_dialog, &PreferencesDialog::slotIconDataChange );
    connect( m_preferences, &Preferences::signalHideOnMinimizeChange, m_pref_dialog, &PreferencesDialog::slotHideOnMinimizeChange );
    connect( m_preferences, &Preferences::signalStartMinimizedChange, m_pref_dialog, &PreferencesDialog::slotStartMinimizedChange );
    connect( m_preferences, &Preferences::signalDebugChange, m_pref_dialog, &PreferencesDialog::slotDebugChange );

    connect( m_preferences, &Preferences::signalIconTypeChange, m_link, &SysTrayXLink::slotIconTypeChange );
    connect( m_preferences, &Preferences::signalIconDataChange, m_link, &SysTrayXLink::slotIconDataChange );
    connect( m_preferences, &Preferences::signalHideOnMinimizeChange, m_link, &SysTrayXLink::slotHideOnMinimizeChange );
    connect( m_preferences, &Preferences::signalStartMinimizedChange, m_link, &SysTrayXLink::slotStartMinimizedChange );
    connect( m_preferences, &Preferences::signalDebugChange, m_link, &SysTrayXLink::slotDebugChange );

    connect( m_preferences, &Preferences::signalDebugChange, m_debug, &DebugWidget::slotDebugChange );

    /*
     *  Connect link signals
     */
    connect( m_link, &SysTrayXLink::signalUnreadMail, m_tray_icon, &SysTrayXIcon::slotSetUnreadMail );
    connect( m_link, &SysTrayXLink::signalAddOnShutdown, this, &SysTrayX::slotAddOnShutdown );
    connect( m_link, &SysTrayXLink::signalWindowState, m_win_ctrl, &WindowCtrl::slotWindowState );
    connect( m_link, &SysTrayXLink::signalTitle, m_win_ctrl, &WindowCtrl::slotWindowTitle );

    /*
     *  Connect window signals
     */
    connect( m_win_ctrl, &WindowCtrl::signalWindowNormal, m_link, &SysTrayXLink::slotWindowNormal );
    connect( m_win_ctrl, &WindowCtrl::signalWindowMinimize, m_link, &SysTrayXLink::slotWindowMinimize );

    /*
     *  Connect system tray signals
     */
    connect( m_tray_icon, &SysTrayXIcon::signalShowHide, m_win_ctrl, &WindowCtrl::slotShowHide );

    /*
     *  SysTrayX
     */
    connect( this, &SysTrayX::signalClose, m_win_ctrl, &WindowCtrl::slotClose );

    /*
     *  Request preferences from add-on
     */
    getPreferences();
}


/*
 *  Send a preferences request
 */
void    SysTrayX::getPreferences()
{
    /*
     *  Request preferences from add-on
     */
    QByteArray request = QString( SysTrayX::JSON_PREF_REQUEST ).toUtf8();
    emit signalWriteMessage( request );
}


/*
 *  Create the actions for the system tray icon menu
 */
void    SysTrayX::createActions()
{
    m_showhide_action = new QAction(tr("&Show/Hide"), this);
    m_showhide_action->setIcon( QIcon( ":/files/icons/window-restore.png" ) );
    connect( m_showhide_action, &QAction::triggered, m_win_ctrl, &WindowCtrl::slotShowHide );

    m_pref_action = new QAction(tr("&Preferences"), this);
    m_pref_action->setIcon( QIcon( ":/files/icons/gtk-preferences.png" ) );
    connect( m_pref_action, &QAction::triggered, m_pref_dialog, &PreferencesDialog::showNormal );

    m_quit_action = new QAction( tr("&Quit"), this );
    m_quit_action->setIcon( QIcon( ":/files/icons/window-close.png" ) );
    connect( m_quit_action, &QAction::triggered, this, &SysTrayX::slotShutdown );
}


/*
 *  Create the system tray icon
 */
void    SysTrayX::createTrayIcon()
{
    /*
     *  Setup menu actions
     */
    createActions();

    /*
     *  Setup menu
     */
    m_tray_icon_menu = new QMenu();

    m_tray_icon_menu->addAction( m_showhide_action );
    m_tray_icon_menu->addSeparator();
    m_tray_icon_menu->addAction( m_pref_action );
    m_tray_icon_menu->addSeparator();
    m_tray_icon_menu->addAction( m_quit_action );

    /*
     *  Create system tray icon
     */
    m_tray_icon = new SysTrayXIcon( m_link, m_preferences );
    m_tray_icon->setContextMenu( m_tray_icon_menu );

    /*
     *  Set icon
     */
    m_tray_icon->setIconMime( m_preferences->getIconMime() );
    m_tray_icon->setIconData( m_preferences->getIconData() );
    m_tray_icon->setIconType( m_preferences->getIconType() );
}


/*
 *  Quit the app by add-on request
 */
void    SysTrayX::slotAddOnShutdown()
{
    /*
     *  Let's quit
     */
    QCoreApplication::quit();
}


/*
 *  Quit the app by quit menu
 */
void    SysTrayX::slotShutdown()
{
    /*
     *  Close the TB window
     */
    emit signalClose();

    /*
     *  Let's quit
     */
    QCoreApplication::quit();
}
