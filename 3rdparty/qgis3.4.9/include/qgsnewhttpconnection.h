/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWHTTPCONNECTION_H
#define QGSNEWHTTPCONNECTION_H

#include "qgis_sip.h"
#include "ui_qgsnewhttpconnectionbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsAuthSettingsWidget;

/**
 * \ingroup gui
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class GUI_EXPORT QgsNewHttpConnection : public QDialog, private Ui::QgsNewHttpConnectionBase
{
    Q_OBJECT

  public:

    /**
     * Available connection types for configuring in the dialog.
     * \since QGIS 3.0
     */
    enum ConnectionType
    {
      ConnectionWfs = 1 << 1, //!< WFS connection
      ConnectionWms = 1 << 2, //!< WMS connection
      ConnectionWcs = 1 << 3, //!< WCS connection
      ConnectionOther = 1 << 4, //!< Other connection type
    };
    Q_DECLARE_FLAGS( ConnectionTypes, ConnectionType )

    /**
     * Flags controlling dialog behavior.
     * \since QGIS 3.0
     */
    enum Flag
    {
      FlagShowTestConnection = 1 << 1, //!< Display the 'test connection' button
      FlagHideAuthenticationGroup = 1 << 2, //!< Hide the Authentication group
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsNewHttpConnection.
     *
     * The \a types argument dictates which connection type settings should be
     * shown in the dialog.
     *
     * The \a flags argument allows specifying flags which control the dialog behavior
     * and appearance.
     */
    QgsNewHttpConnection( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                          QgsNewHttpConnection::ConnectionTypes types = ConnectionWms,
                          const QString &baseKey = "qgis/connections-wms/",
                          const QString &connectionName = QString(),
                          QgsNewHttpConnection::Flags flags = nullptr,
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Returns the current connection name.
     * \since QGIS 3.0
     */
    QString name() const;

    /**
     * Returns the current connection url.
     * \since QGIS 3.0
     */
    QString url() const;

  public slots:

    void accept() override;

  private slots:

    void nameChanged( const QString & );
    void urlChanged( const QString & );
    void updateOkButtonState();
    void wfsVersionCurrentIndexChanged( int index );
    void wfsFeaturePagingStateChanged( int state );

  protected:

    /**
     * Returns true if dialog settings are valid, or false if current
     * settings are not valid and the dialog should not be acceptable.
     * \since QGIS 3.0
     */
    virtual bool validate();

    /**
     * Returns the "test connection" button.
     * \since QGIS 3.0
     */
    QPushButton *testConnectButton();

    /**
     * Returns the current authentication settings widget.
     * \since QGIS 3.8
     */
    QgsAuthSettingsWidget *authSettingsWidget() SIP_SKIP;

    /**
     * Returns the "WFS version detect" button.
     * \since QGIS 3.2
     */
    QPushButton *wfsVersionDetectButton() SIP_SKIP;

    /**
     * Returns the "WFS version" combobox.
     * \since QGIS 3.2
     */
    QComboBox *wfsVersionComboBox() SIP_SKIP;

    /**
     * Returns the "WFS paging enabled" checkbox
     * \since QGIS 3.2
     */
    QCheckBox *wfsPagingEnabledCheckBox() SIP_SKIP;

    /**
     * Returns the "WFS page size" edit
     * \since QGIS 3.2
     */
    QLineEdit *wfsPageSizeLineEdit() SIP_SKIP;

    /**
     * Returns the url.
     * \since QGIS 3.2
     */
    QUrl urlTrimmed() const SIP_SKIP;

    /**
     * Returns the QSettings key for WFS related settings for the connection.
     * \see wmsSettingsKey()
     * \since QGIS 3.0
     */
    virtual QString wfsSettingsKey( const QString &base, const QString &connectionName ) const;

    /**
     * Returns the QSettings key for WMS related settings for the connection.
     * \see wfsSettingsKey()
     * \since QGIS 3.0
     */
    virtual QString wmsSettingsKey( const QString &base, const QString &connectionName ) const;

    /**
     * Triggers a resync of the GUI widgets for the service specific settings (i.e. WFS
     * and WMS related settings).
     * \since QGIS 3.0
     */
    void updateServiceSpecificSettings();

  private:

    ConnectionTypes mTypes = ConnectionWms;

    QString mBaseKey;
    QString mCredentialsBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNewHttpConnection::ConnectionTypes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNewHttpConnection::Flags )

// clazy:excludeall=qstring-allocations

#endif //  QGSNEWHTTPCONNECTION_H
