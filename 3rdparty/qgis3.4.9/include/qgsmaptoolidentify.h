/***************************************************************************
    qgsmaptoolidentify.h  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLIDENTIFY_H
#define QGSMAPTOOLIDENTIFY_H

#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgsunittypes.h"

#include <QObject>
#include <QPointer>
#include "qgis_gui.h"

class QgsRasterLayer;
class QgsVectorLayer;
class QgsMapLayer;
class QgsMapCanvas;
class QgsHighlight;
class QgsIdentifyMenu;

/**
 * \ingroup gui
  \brief Map tool for identifying features in layers

  after selecting a point, performs the identification:
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows editing values when vector layer is in editing mode)
*/
class GUI_EXPORT QgsMapToolIdentify : public QgsMapTool
{
    Q_OBJECT

  public:

    enum IdentifyMode
    {
      DefaultQgsSetting = -1,
      ActiveLayer,
      TopDownStopAtFirst,
      TopDownAll,
      LayerSelection
    };
    Q_ENUM( IdentifyMode )

    enum Type
    {
      VectorLayer = 1,
      RasterLayer = 2,
      AllLayers = VectorLayer | RasterLayer
    };
    Q_DECLARE_FLAGS( LayerType, Type )
    Q_FLAG( LayerType )

    struct IdentifyResult
    {
      //! Constructor for IdentifyResult
      IdentifyResult() = default;

      IdentifyResult( QgsMapLayer *layer, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer *layer, const QString &label, const QMap< QString, QString > &attributes, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mLabel( label ), mAttributes( attributes ), mDerivedAttributes( derivedAttributes ) {}

      IdentifyResult( QgsMapLayer *layer, const QString &label, const QgsFields &fields, const QgsFeature &feature, const QMap< QString, QString > &derivedAttributes )
        : mLayer( layer ), mLabel( label ), mFields( fields ), mFeature( feature ), mDerivedAttributes( derivedAttributes ) {}

      QgsMapLayer *mLayer = nullptr;
      QString mLabel;
      QgsFields mFields;
      QgsFeature mFeature;
      QMap< QString, QString > mAttributes;
      QMap< QString, QString > mDerivedAttributes;
      QMap< QString, QVariant > mParams;
    };

    //! constructor
    QgsMapToolIdentify( QgsMapCanvas *canvas );

    ~QgsMapToolIdentify() override;

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void deactivate() override;

    /**
     * Performs the identification.
    \param x x coordinates of mouseEvent
    \param y y coordinates of mouseEvent
    \param layerList Performs the identification within the given list of layers. Default value is an empty list, i.e. uses all the layers.
    \param mode Identification mode. Can use Qgis default settings or a defined mode. Default mode is DefaultQgsSetting.
    \returns a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, const QList<QgsMapLayer *> &layerList = QList<QgsMapLayer *>(), IdentifyMode mode = DefaultQgsSetting );

    /**
     * Performs the identification.
    To avoid being forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    \param x x coordinates of mouseEvent
    \param y y coordinates of mouseEvent
    \param mode Identification mode. Can use Qgis default settings or a defined mode.
    \param layerType Only performs identification in a certain type of layers (raster, vector). Default value is AllLayers.
    \returns a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode, LayerType layerType = AllLayers );

    //! Performs identification based on a geometry (in map coordinates)
    QList<QgsMapToolIdentify::IdentifyResult> identify( const QgsGeometry &geometry, IdentifyMode mode, LayerType layerType );
    //! Performs identification based on a geometry (in map coordinates)
    QList<QgsMapToolIdentify::IdentifyResult> identify( const QgsGeometry &geometry, IdentifyMode mode, const QList<QgsMapLayer *> &layerList, LayerType layerType );


    /**
     * Returns a pointer to the identify menu which will be used in layer selection mode
     * this menu can also be customized
     */
    QgsIdentifyMenu *identifyMenu() {return mIdentifyMenu;}

  public slots:
    void formatChanged( QgsRasterLayer *layer );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( const QString & );
    void changedRasterResults( QList<QgsMapToolIdentify::IdentifyResult> & );

  protected:

    /**
     * Performs the identification.
    To avoid being forced to specify IdentifyMode with a list of layers
    this has been made private and two publics methods are offered
    \param x x coordinates of mouseEvent
    \param y y coordinates of mouseEvent
    \param mode Identification mode. Can use Qgis default settings or a defined mode.
    \param layerList Performs the identification within the given list of layers.
    \param layerType Only performs identification in a certain type of layers (raster, vector).
    \returns a list of IdentifyResult*/
    QList<QgsMapToolIdentify::IdentifyResult> identify( int x, int y, IdentifyMode mode,  const QList<QgsMapLayer *> &layerList, LayerType layerType = AllLayers );

    QgsIdentifyMenu *mIdentifyMenu = nullptr;

    //! Call the right method depending on layer type
    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsPointXY &point, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType = AllLayers );

    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, QgsPointXY point, const QgsRectangle &viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsPointXY &point );

    //! Returns derived attributes map for a clicked point in map coordinates. May be 2D or 3D point.
    QMap< QString, QString > derivedAttributesForPoint( const QgsPoint &point );

    /**
     * Overrides some map canvas properties inside the map tool for the upcoming identify requests.
     *
     * This is useful when the identification is triggered by some other piece of GUI like a 3D map view
     * and some properties like search radius need to be adjusted so that identification returns correct
     * results. Currently only search radius may be overridden.
     *
     * When the custom identification has finished, restoreCanvasPropertiesOverrides() should
     * be called to erase any overrides.
     * \see restoreCanvasPropertiesOverrides()
     * \since QGIS 3.4
     */
    void setCanvasPropertiesOverrides( double searchRadiusMapUnits );

    /**
     * Clears canvas properties overrides previously set with setCanvasPropertiesOverrides()
     * \see setCanvasPropertiesOverrides()
     * \since QGIS 3.4
     */
    void restoreCanvasPropertiesOverrides();

  private:

    bool identifyLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsMapLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel, QgsMapToolIdentify::LayerType layerType = AllLayers );
    bool identifyRasterLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsRasterLayer *layer, const QgsGeometry &geometry, const QgsRectangle &viewExtent, double mapUnitsPerPixel );
    bool identifyVectorLayer( QList<QgsMapToolIdentify::IdentifyResult> *results, QgsVectorLayer *layer, const QgsGeometry &geometry );

    /**
     * Desired units for distance display.
     * \see displayAreaUnits()
     * \since QGIS 2.14
     */
    virtual QgsUnitTypes::DistanceUnit displayDistanceUnits() const;

    /**
     * Desired units for area display.
     * \see displayDistanceUnits()
     * \since QGIS 2.14
     */
    virtual QgsUnitTypes::AreaUnit displayAreaUnits() const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatArea()
     * \since QGIS 2.14
     */
    QString formatDistance( double distance ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatDistance()
     * \since QGIS 2.14
     */
    QString formatArea( double area ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatArea()
     */
    QString formatDistance( double distance, QgsUnitTypes::DistanceUnit unit ) const;

    /**
     * Format a distance into a suitable string for display to the user
     * \see formatDistance()
     */
    QString formatArea( double area, QgsUnitTypes::AreaUnit unit ) const;

    QMap< QString, QString > featureDerivedAttributes( const QgsFeature &feature, QgsMapLayer *layer, const QgsPointXY &layerPoint = QgsPointXY() );

    /**
     * Adds details of the closest vertex to derived attributes
     */
    void closestVertexAttributes( const QgsAbstractGeometry &geometry, QgsVertexId vId, QgsMapLayer *layer, QMap< QString, QString > &derivedAttributes );

    /**
     * Adds details of the closest point to derived attributes
    */
    void closestPointAttributes( const QgsAbstractGeometry &geometry, const QgsPointXY &layerPoint, QMap< QString, QString > &derivedAttributes );

    QString formatCoordinate( const QgsPointXY &canvasPoint ) const;
    QString formatXCoordinate( const QgsPointXY &canvasPoint ) const;
    QString formatYCoordinate( const QgsPointXY &canvasPoint ) const;

    // Last geometry (point or polygon) in map CRS
    QgsGeometry mLastGeometry;

    double mLastMapUnitsPerPixel;

    QgsRectangle mLastExtent;

    int mCoordinatePrecision;

    double mOverrideCanvasSearchRadius = -1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapToolIdentify::LayerType )

#endif
