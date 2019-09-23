
/***************************************************************************
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H


#include "qgis_core.h"
#include <QMap>
#include <QSet>
#include <QList>
#include <QStringList>
#include <QFont>
#include <QMutex>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfeaturesource.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorsimplifymethod.h"
#include "qgseditformconfig.h"
#include "qgsattributetableconfig.h"
#include "qgsaggregatecalculator.h"
#include "qgsfeatureiterator.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsexpressioncontextscopegenerator.h"

class QPainter;
class QImage;

class QgsAbstractGeometrySimplifier;
class QgsActionManager;
class QgsConditionalLayerStyles;
class QgsCurve;
class QgsDiagramLayerSettings;
class QgsDiagramRenderer;
class QgsEditorWidgetWrapper;
class QgsExpressionFieldBuffer;
class QgsFeatureRenderer;
class QgsGeometry;
class QgsGeometryVertexIndex;
class QgsMapToPixel;
class QgsRectangle;
class QgsRectangle;
class QgsRelation;
class QgsRelationManager;
class QgsSingleSymbolRenderer;
class QgsSymbol;
class QgsVectorLayerJoinInfo;
class QgsVectorLayerEditBuffer;
class QgsVectorLayerJoinBuffer;
class QgsVectorLayerFeatureCounter;
class QgsAbstractVectorLayerLabeling;
class QgsPoint;
class QgsFeedback;
class QgsAuxiliaryStorage;
class QgsAuxiliaryLayer;
class QgsGeometryOptions;

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;


/**
 * \ingroup core
 * Represents a vector layer which manages a vector based data sets.
 *
 * The QgsVectorLayer is instantiated by specifying the name of a data provider,
 * such as postgres or wfs, and url defining the specific data set to connect to.
 * The vector layer constructor in turn instantiates a QgsVectorDataProvider subclass
 * corresponding to the provider type, and passes it the url. The data provider
 * connects to the data source.
 *
 * The QgsVectorLayer provides a common interface to the different data types. It also
 * manages editing transactions.
 *
 *  Sample usage of the QgsVectorLayer class:
 *
 * \code
 *     QString uri = "point?crs=epsg:4326&field=id:integer";
 *     QgsVectorLayer *scratchLayer = new QgsVectorLayer(uri, "Scratch point layer",  "memory");
 * \endcode
 *
 * The main data providers supported by QGIS are listed below.
 *
 * \section providers Vector data providers
 *
 * \subsection memory Memory data providerType (memory)
 *
 * The memory data provider is used to construct in memory data, for example scratch
 * data or data generated from spatial operations such as contouring. There is no
 * inherent persistent storage of the data. The data source uri is constructed. The
 * url specifies the geometry type ("point", "linestring", "polygon",
 * "multipoint","multilinestring","multipolygon"), optionally followed by url parameters
 * as follows:
 *
 * - crs=definition
 *   Defines the coordinate reference system to use for the layer.
 *   definition is any string accepted by QgsCoordinateReferenceSystem::createFromString()
 *
 * - index=yes
 *   Specifies that the layer will be constructed with a spatial index
 *
 * - field=name:type(length,precision)
 *   Defines an attribute of the layer. Multiple field parameters can be added
 *   to the data provider definition. type is one of "integer", "double", "string".
 *
 * An example url is "Point?crs=epsg:4326&field=id:integer&field=name:string(20)&index=yes"
 *
 * Since QGIS 3.4 when closing a project, the application shows a warning about potential data
 * loss if there are any non-empty memory layers present. If your memory layer should not
 * trigger such warning, it is possible to suppress that by setting the following custom variable:
 * \code{.py}
 *   layer.setCustomProperty("skipMemoryLayersCheck", 1)
 * \endcode
 *
 *
 * \subsection ogr OGR data provider (ogr)
 *
 * Accesses data using the OGR drivers (http://www.gdal.org/ogr/ogr_formats.html). The url
 * is the OGR connection string. A wide variety of data formats can be accessed using this
 * driver, including file based formats used by many GIS systems, database formats, and
 * web services. Some of these formats are also supported by custom data providers listed
 * below.
 *
 * \subsection spatialite SpatiaLite data provider (spatialite)
 *
 * Access data in a SpatiaLite database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection postgres PostgreSQL data provider (postgres)
 *
 * Connects to a PostgreSQL database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection mssql Microsoft SQL server data provider (mssql)
 *
 * Connects to a Microsoft SQL server database. The url defines the connection parameters, table,
 * geometry column, and other attributes. The url can be constructed using the
 * QgsDataSourceUri class.
 *
 * \subsection wfs WFS (web feature service) data provider (wfs)
 *
 * Used to access data provided by a web feature service.
 *
 * The url can be a HTTP url to a WFS server (legacy, e.g. http://foobar/wfs?TYPENAME=xxx&SRSNAME=yyy[&FILTER=zzz]), or,
 * starting with QGIS 2.16, a URI constructed using the QgsDataSourceUri class with the following parameters :
 * - url=string (mandatory): HTTP url to a WFS server endpoint. e.g http://foobar/wfs
 * - typename=string (mandatory): WFS typename
 * - srsname=string (recommended): SRS like 'EPSG:XXXX'
 * - username=string
 * - password=string
 * - authcfg=string
 * - version=auto/1.0.0/1.1.0/2.0.0
 *  -sql=string: full SELECT SQL statement with optional WHERE, ORDER BY and possibly with JOIN if supported on server
 * - filter=string: QGIS expression or OGC/FES filter
 * - restrictToRequestBBOX=1: to download only features in the view extent (or more generally
 *   in the bounding box of the feature iterator)
 * - maxNumFeatures=number
 * - IgnoreAxisOrientation=1: to ignore EPSG axis order for WFS 1.1 or 2.0
 * - InvertAxisOrientation=1: to invert axis order
 * - hideDownloadProgressDialog=1: to hide the download progress dialog
 *
 * The ‘FILTER’ query string parameter can be used to filter
 * the WFS feature type. The ‘FILTER’ key value can either be a QGIS expression
 * or an OGC XML filter. If the value is set to a QGIS expression the driver will
 * turn it into OGC XML filter before passing it to the WFS server. Beware the
 * QGIS expression filter only supports” =, !=, <, >, <=, >=, AND, OR, NOT, LIKE, IS NULL”
 * attribute operators, “BBOX, Disjoint, Intersects, Touches, Crosses, Contains, Overlaps, Within”
 * spatial binary operators and the QGIS local “geomFromWKT, geomFromGML”
 * geometry constructor functions.
 *
 * Also note:
 *
 * - You can use various functions available in the QGIS Expression list,
 *   however the function must exist server side and have the same name and arguments to work.
 *
 * - Use the special $geometry parameter to provide the layer geometry column as input
 *   into the spatial binary operators e.g intersects($geometry, geomFromWKT('POINT (5 6)'))
 *
 * \subsection delimitedtext Delimited text file data provider (delimitedtext)
 *
 * Accesses data in a delimited text file, for example CSV files generated by
 * spreadsheets. The contents of the file are split into columns based on specified
 * delimiter characters.  Each record may be represented spatially either by an
 * X and Y coordinate column, or by a WKT (well known text) formatted columns.
 *
 * The url defines the filename, the formatting options (how the
 * text in the file is divided into data fields, and which fields contain the
 * X,Y coordinates or WKT text definition.  The options are specified as url query
 * items.
 *
 * At its simplest the url can just be the filename, in which case it will be loaded
 * as a CSV formatted file.
 *
 * The url may include the following items:
 *
 * - encoding=UTF-8
 *
 *   Defines the character encoding in the file.  The default is UTF-8.  To use
 *   the default encoding for the operating system use "System".
 *
 * - type=(csv|regexp|whitespace|plain)
 *
 *   Defines the algorithm used to split records into columns. Records are
 *   defined by new lines, except for csv format files for which quoted fields
 *   may span multiple records.  The default type is csv.
 *
 *   -  "csv" splits the file based on three sets of characters:
 *      delimiter characters, quote characters,
 *      and escape characters.  Delimiter characters mark the end
 *      of a field. Quote characters enclose a field which can contain
 *      delimiter characters, and newlines.  Escape characters cause the
 *      following character to be treated literally (including delimiter,
 *      quote, and newline characters).  Escape and quote characters must
 *      be different from delimiter characters. Escape characters that are
 *      also quote characters are treated specially - they can only
 *      escape themselves within quotes.  Elsewhere they are treated as
 *      quote characters.  The defaults for delimiter, quote, and escape
 *      are ',', '"', '"'.
 *   -  "regexp" splits each record using a regular expression (see QRegExp
 *      documentation for details).
 *   -  "whitespace" splits each record based on whitespace (on or more whitespace
 *      characters.  Leading whitespace in the record is ignored.
 *   -  "plain" is provided for backwards compatibility.  It is equivalent to
 *      CSV except that the default quote characters are single and double quotes,
 *      and there is no escape characters.
 *
 * - delimiter=characters
 *
 *   Defines the delimiter characters used for csv and plain type files, or the
 *   regular expression for regexp type files.  It is a literal string of characters
 *   except that "\t" may be used to represent a tab character.
 *
 * - quote=characters
 *
 *   Defines the characters that are used as quote characters for csv and plain type
 *   files.
 *
 * - escape=characters
 *
 *   Defines the characters used to escape delimiter, quote, and newline characters.
 *
 * - skipLines=n
 *
 *   Defines the number of lines to ignore at the beginning of the file (default 0)
 *
 * - useHeader=(yes|no)
 *
 *   Defines whether the first record in the file (after skipped lines) contains
 *   column names (default yes)
 *
 * - trimFields=(yes|no)
 *
 *   If yes then leading and trailing whitespace will be removed from fields
 *
 * - skipEmptyFields=(yes|no)
 *
 *   If yes then empty fields will be discarded (equivalent to concatenating consecutive
 *   delimiters)
 *
 * - maxFields=#
 *
 *   Specifies the maximum number of fields to load for each record.  Additional
 *   fields will be discarded.  Default is 0 - load all fields.
 *
 * - decimalPoint=c
 *
 *   Defines a character that is used as a decimal point in the numeric columns
 *   The default is '.'.
 *
 * - xField=column yField=column
 *
 *   Defines the name of the columns holding the x and y coordinates for XY point geometries.
 *   If the useHeader is no (ie there are no column names), then this is the column
 *   number (with the first column as 1).
 *
 * - xyDms=(yes|no)
 *
 *   If yes then the X and Y coordinates are interpreted as
 *   degrees/minutes/seconds format (fairly permissively),
 *   or degree/minutes format.
 *
 * - wktField=column
 *
 *   Defines the name of the columns holding the WKT geometry definition for WKT geometries.
 *   If the useHeader is no (ie there are no column names), then this is the column
 *   number (with the first column as 1).
 *
 * - geomType=(point|line|polygon|none)
 *
 *   Defines the geometry type for WKT type geometries.  QGIS will only display one
 *   type of geometry for the layer - any others will be ignored when the file is
 *   loaded.  By default the provider uses the type of the first geometry in the file.
 *   Use geomType to override this type.
 *
 *   geomType can also be set to none, in which case the layer is loaded without
 *   geometries.
 *
 * - subset=expression
 *
 *   Defines an expression that will identify a subset of records to display
 *
 * - crs=crsstring
 *
 *   Defines the coordinate reference system used for the layer.  This can be
 *   any string accepted by QgsCoordinateReferenceSystem::createFromString()
 *
 * -subsetIndex=(yes|no)
 *
 *   Determines whether the provider generates an index to improve the efficiency
 *   of subsets.  The default is yes
 *
 * -spatialIndex=(yes|no)
 *
 *   Determines whether the provider generates a spatial index.  The default is no.
 *
 * -watchFile=(yes|no)
 *
 *   Defines whether the file will be monitored for changes. The default is
 *   to monitor for changes.
 *
 * - quiet
 *
 *   Errors encountered loading the file will not be reported in a user dialog if
 *   quiet is included (They will still be shown in the output log).
 *
 * \subsection gpx GPX data provider (gpx)
 *
 * Provider reads tracks, routes, and waypoints from a GPX file.  The url
 * defines the name of the file, and the type of data to retrieve from it
 * ("track", "route", or "waypoint").
 *
 * An example url is "/home/user/data/holiday.gpx?type=route"
 *
 * \subsection grass Grass data provider (grass)
 *
 * Provider to display vector data in a GRASS GIS layer.
 *
 * TODO QGIS3: Remove virtual from non-inherited methods (like isModified)
 * \see QgsVectorLayerUtils()
 */
class CORE_EXPORT QgsVectorLayer : public QgsMapLayer, public QgsExpressionContextGenerator, public QgsExpressionContextScopeGenerator, public QgsFeatureSink, public QgsFeatureSource
{
    Q_OBJECT

    Q_PROPERTY( QString subsetString READ subsetString WRITE setSubsetString NOTIFY subsetStringChanged )
    Q_PROPERTY( QString displayExpression READ displayExpression WRITE setDisplayExpression NOTIFY displayExpressionChanged )
    Q_PROPERTY( QString mapTipTemplate READ mapTipTemplate WRITE setMapTipTemplate NOTIFY mapTipTemplateChanged )
    Q_PROPERTY( QgsEditFormConfig editFormConfig READ editFormConfig WRITE setEditFormConfig NOTIFY editFormConfigChanged )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged )
    Q_PROPERTY( double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged )

  public:

    //! Result of an edit operation
    enum EditResult
    {
      Success = 0, //!< Edit operation was successful
      EmptyGeometry = 1, //!< Edit operation resulted in an empty geometry
      EditFailed = 2, //!< Edit operation failed
      FetchFeatureFailed = 3, //!< Unable to fetch requested feature
      InvalidLayer = 4, //!< Edit failed due to invalid layer
    };

    //! Selection behavior
    enum SelectBehavior
    {
      SetSelection, //!< Set selection, removing any existing selection
      AddToSelection, //!< Add selection to current selection
      IntersectSelection, //!< Modify current selection to include only select features which match
      RemoveFromSelection, //!< Remove from current selection
    };

    /**
     * Setting options for loading vector layers.
     * \since QGIS 3.0
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( bool loadDefaultStyle = true, bool readExtentFromXml = false )
        : loadDefaultStyle( loadDefaultStyle )
        , readExtentFromXml( readExtentFromXml )
      {}

      //! Sets to true if the default layer style should be loaded
      bool loadDefaultStyle = true;

      /**
       * If true, the layer extent will be read from XML (i.e. stored in the
       * project file). If false, the extent will be determined by the provider on layer load.
       */
      bool readExtentFromXml = false;

    };

    /**
     * Constructor - creates a vector layer
     *
     * The QgsVectorLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * \param path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * \param baseName The name used to represent the layer in the legend
     * \param providerLib  The name of the data provider, e.g., "memory", "postgres"
     * \param options layer load options
     */
    explicit QgsVectorLayer( const QString &path = QString(), const QString &baseName = QString(),
                             const QString &providerLib = "ogr", const QgsVectorLayer::LayerOptions &options = QgsVectorLayer::LayerOptions() );


    ~QgsVectorLayer() override;

    //! QgsVectorLayer cannot be copied.
    QgsVectorLayer( const QgsVectorLayer &rhs ) = delete;
    //! QgsVectorLayer cannot be copied.
    QgsVectorLayer &operator=( QgsVectorLayer const &rhs ) = delete;

    /**
     * Returns a new instance equivalent to this one. A new provider is
     *  created for the same data source and renderers for features and diagrams
     *  are cloned too. Moreover, each attributes (transparency, extent, selected
     *  features and so on) are identicals.
     * \returns a new layer instance
     * \since QGIS 3.0
     */
    QgsVectorLayer *clone() const override SIP_FACTORY;

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     * This is obtained from the data provider and does not follow any standard.
     */
    QString storageType() const;

    /**
     * Capabilities for this layer, comma separated and translated.
     */
    QString capabilitiesString() const;

    /**
     * Returns a description for this layer as defined in the data provider.
     */
    QString dataComment() const;

    /**
     * This is a shorthand for accessing the displayExpression if it is a simple field.
     * If the displayExpression is more complex than a simple field, a null string will
     * be returned.
     *
     * \see displayExpression
     */
    QString displayField() const;

    /**
     * Set the preview expression, used to create a human readable preview string.
     *  Used e.g. in the attribute table feature list. Uses QgsExpression.
     *
     *  \param displayExpression The expression which will be used to preview features
     *                           for this layer
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     *  Returns the preview expression, used to create a human readable preview string.
     *  Uses QgsExpression
     *
     *  \returns The expression which will be used to preview features for this layer
     */
    QString displayExpression() const;

    QgsVectorDataProvider *dataProvider() FINAL;
    const QgsVectorDataProvider *dataProvider() const FINAL SIP_SKIP;

    //! Sets the textencoding of the data provider
    void setProviderEncoding( const QString &encoding );

    //! Setup the coordinate system transformation for the layer
    void setCoordinateSystem();

    /**
     * Joins another vector layer to this layer
      \param joinInfo join object containing join layer id, target and source field
      \note since 2.6 returns bool indicating whether the join can be added */
    bool addJoin( const QgsVectorLayerJoinInfo &joinInfo );

    /**
     * Removes a vector layer join
      \returns true if join was found and successfully removed */
    bool removeJoin( const QString &joinLayerId );

    /**
     * Returns the join buffer object.
     * \since QGIS 2.14.7
     */
    QgsVectorLayerJoinBuffer *joinBuffer() { return mJoinBuffer; }
    const QList<QgsVectorLayerJoinInfo> vectorJoins() const;

    /**
     * Sets the list of dependencies.
     * \see dependencies()
     *
     * \param layers set of QgsMapLayerDependency. Only user-defined dependencies will be added
     * \returns false if a dependency cycle has been detected
     * \since QGIS 3.0
     */
    bool setDependencies( const QSet<QgsMapLayerDependency> &layers ) FINAL;

    /**
     * Gets the list of dependencies. This includes data dependencies set by the user (\see setDataDependencies)
     * as well as dependencies given by the provider
     *
     * \returns a set of QgsMapLayerDependency
     * \since QGIS 3.0
     */
    QSet<QgsMapLayerDependency> dependencies() const FINAL;

    /**
     * Add a new field which is calculated by the expression specified
     *
     * \param exp The expression which calculates the field
     * \param fld The field to calculate
     *
     * \returns The index of the new field
     *
     * \since QGIS 2.9
     */
    int addExpressionField( const QString &exp, const QgsField &fld );

    /**
     * Remove an expression field
     *
     * \param index The index of the field
     *
     * \since QGIS 2.6
     */
    void removeExpressionField( int index );

    /**
     * Returns the expression used for a given expression field
     *
     * \param index An index of an epxression based (virtual) field
     *
     * \returns The expression for the field at index
     *
     * \since QGIS 2.9
     */
    QString expressionField( int index ) const;

    /**
     * Changes the expression used to define an expression based (virtual) field
     *
     * \param index The index of the expression to change
     *
     * \param exp The new expression to set
     *
     * \since QGIS 2.9
     */
    void updateExpressionField( int index, const QString &exp );

    /**
     * Returns all layer actions defined on this layer.
     *
     * The pointer which is returned directly points to the actions object
     * which is used by the layer, so any changes are immediately applied.
     */
    QgsActionManager *actions() { return mActions; }

    /**
     * Returns all layer actions defined on this layer.
     *
     * The pointer which is returned is const.
     */
    const QgsActionManager *actions() const SIP_SKIP { return mActions; }

    /**
     * Returns the number of features that are selected in this layer.
     *
     * \see selectedFeatureIds()
     */
    int selectedFeatureCount() const;

    /**
     * Select features found within the search rectangle (in layer's coordinates)
     * \param rect search rectangle
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see invertSelectionInRectangle(QgsRectangle & rect)
     * \see selectByExpression()
     * \see selectByIds()
     */
    void selectByRect( QgsRectangle &rect, SelectBehavior behavior = SetSelection );

    /**
     * Select matching features using an expression.
     * \param expression expression to evaluate to select features
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see selectByRect()
     * \see selectByIds()
     * \since QGIS 2.16
     */
    void selectByExpression( const QString &expression, SelectBehavior behavior = SetSelection );

    /**
     * Select matching features using a list of feature IDs. Will emit the
     * selectionChanged() signal with the clearAndSelect flag set.
     * \param ids feature IDs to select
     * \param behavior selection type, allows adding to current selection, removing
     * from selection, etc.
     * \see selectByRect()
     * \see selectByExpression()
     * \since QGIS 2.16
     */
    void selectByIds( const QgsFeatureIds &ids, SelectBehavior behavior = SetSelection );

    /**
     * Modifies the current selection on this layer
     *
     * \param selectIds    Select these ids
     * \param deselectIds  Deselect these ids
     *
     * \see   selectByIds
     * \see   deselect(const QgsFeatureIds&)
     * \see   deselect(const QgsFeatureId)
     * \see selectByExpression()
     */
    void modifySelection( const QgsFeatureIds &selectIds, const QgsFeatureIds &deselectIds );

    //! Select not selected features and deselect selected ones
    void invertSelection();

    //! Select all the features
    void selectAll();

    /**
     * Invert selection of features found within the search rectangle (in layer's coordinates)
     *
     * \param rect  The rectangle in which the selection of features will be inverted
     *
     * \see   invertSelection()
     */
    void invertSelectionInRectangle( QgsRectangle &rect );

    /**
     * Returns a copy of the user-selected features.
     *
     * \warning Calling this method triggers a request for all attributes and geometry for the selected features.
     * Consider using the much more efficient selectedFeatureIds() or selectedFeatureCount() if you do not
     * require access to the feature attributes or geometry.
     *
     * \returns A list of QgsFeature
     *
     * \see    selectedFeatureIds()
     * \see    getSelectedFeatures() which is more memory friendly when handling large selections
     */
    QgsFeatureList selectedFeatures() const;

    /**
     * Returns an iterator of the selected features.
     *
     * \param request You may specify a request, e.g. to limit the set of requested attributes.
     *                Any filter on the request will be discarded.
     *
     * \returns Iterator over the selected features
     *
     * \warning Calling this method returns an iterator for all attributes and geometry for the selected features.
     * Consider using the much more efficient selectedFeatureIds() or selectedFeatureCount() if you do not
     * require access to the feature attributes or geometry.
     *
     * \see    selectedFeatureIds()
     * \see    selectedFeatures()
     */
    QgsFeatureIterator getSelectedFeatures( QgsFeatureRequest request = QgsFeatureRequest() ) const;

    /**
     * Returns a list of the selected features IDs in this layer.
     *
     * \see selectedFeatures()
     * \see selectedFeatureCount()
     */
    const QgsFeatureIds &selectedFeatureIds() const;

    //! Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned
    QgsRectangle boundingBoxOfSelected() const;

    /**
     * Returns whether the layer contains labels which are enabled and should be drawn.
     * \returns true if layer contains enabled labels
     *
     * \see setLabelsEnabled()
     * \since QGIS 2.9
     */
    bool labelsEnabled() const;

    /**
     * Sets whether labels should be \a enabled for the layer.
     *
     * \note Labels will only be rendered if labelsEnabled() is true and a labeling
     * object is returned by labeling().
     *
     * \see labelsEnabled()
     * \see labeling()
     */
    void setLabelsEnabled( bool enabled );

    /**
     * Returns whether the layer contains diagrams which are enabled and should be drawn.
     * \returns true if layer contains enabled diagrams
     * \since QGIS 2.9
     */
    bool diagramsEnabled() const;

    //! Sets diagram rendering object (takes ownership)
    void setDiagramRenderer( QgsDiagramRenderer *r SIP_TRANSFER );
    const QgsDiagramRenderer *diagramRenderer() const { return mDiagramRenderer; }

    void setDiagramLayerSettings( const QgsDiagramLayerSettings &s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const { return mDiagramLayerSettings; }

    //! Returns renderer.
    QgsFeatureRenderer *renderer() { return mRenderer; }

    /**
     * Returns const renderer.
     * \note not available in Python bindings
     */
    const QgsFeatureRenderer *renderer() const SIP_SKIP { return mRenderer; }

    /**
     * Set renderer which will be invoked to represent this layer.
     * Ownership is transferred.
     */
    void setRenderer( QgsFeatureRenderer *r SIP_TRANSFER );

    //! Returns point, line or polygon
    QgsWkbTypes::GeometryType geometryType() const;

    //! Returns the WKBType or WKBUnknown in case of error
    QgsWkbTypes::Type wkbType() const FINAL;

    //! Returns the provider type for this layer
    QString providerType() const;

    QgsCoordinateReferenceSystem sourceCrs() const FINAL;
    QString sourceName() const FINAL;

    /**
     * Reads vector layer specific state from project file Dom node.
     * \note Called by QgsMapLayer::readXml().
     */
    bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context ) FINAL;

    /**
     * Write vector layer specific state to project file Dom node.
     * \note Called by QgsMapLayer::writeXml().
     */
    bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const FINAL;

    QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const FINAL;
    QString decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const FINAL;

    /**
     * Resolve references to other layers (kept as layer IDs after reading XML) into layer objects.
     * \since QGIS 3.0
     */
    void resolveReferences( QgsProject *project ) FINAL;

    /**
     * Save named and sld style of the layer to the style table in the db.
     * \param name
     * \param description
     * \param useAsDefault
     * \param uiFileContent
     * \param msgError
     */
    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError SIP_OUT );

    /**
     * Lists all the style in db split into related to the layer and not related to
     * \param ids the list in which will be stored the style db ids
     * \param names the list in which will be stored the style names
     * \param descriptions the list in which will be stored the style descriptions
     * \param msgError
     * \returns the number of styles related to current layer
     * \note Since QGIS 3.2 Styles related to the layer are ordered with the default style first then by update time for Postgres, MySQL and Spatialite.
     */
    virtual int listStylesInDatabase( QStringList &ids SIP_OUT, QStringList &names SIP_OUT,
                                      QStringList &descriptions SIP_OUT, QString &msgError SIP_OUT );

    /**
     * Will return the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Delete a style from the database
     * \param styleId the provider's layer_styles table id of the style to delete
     * \param msgError reference to string that will be updated with any error messages
     * \returns true in case of success
     * \since QGIS 3.0
     */
    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Load a named style from file/local db/datasource db
     * \param theURI the URI of the style or the URI of the layer
     * \param resultFlag will be set to true if a named style is correctly loaded
     * \param loadFromLocalDb if true forces to load from local db instead of datasource one
     * \param categories the style categories to be loaded.
     */
    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag SIP_OUT, bool loadFromLocalDb,
                                    QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories );

    /**
     * Calls loadNamedStyle( theURI, resultFlag, false );
     * Retained for backward compatibility
     */
    QString loadNamedStyle( const QString &theURI, bool &resultFlag SIP_OUT,
                            QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;

    /**
     * Loads the auxiliary layer for this vector layer. If there's no
     * corresponding table in the database, then nothing happens and false is
     * returned. The key is optional because if this layer has been read from
     * a XML document, then the key read in this document is used by default.
     *
     * \param storage The auxiliary storage where to look for the table
     * \param key The key to use for joining.
     *
     * \returns true if the auxiliary layer is well loaded, false otherwise
     *
     * \since QGIS 3.0
     */
    bool loadAuxiliaryLayer( const QgsAuxiliaryStorage &storage, const QString &key = QString() );

    /**
     * Sets the current auxiliary layer. The auxiliary layer is automatically
     * put in editable mode and fields are updated. Moreover, a join is created
     * between the current layer and the auxiliary layer. Ownership is
     * transferred.
     *
     *
     * \since QGIS 3.0
     */
    void setAuxiliaryLayer( QgsAuxiliaryLayer *layer SIP_TRANSFER = nullptr );

    /**
     * Returns the current auxiliary layer.
     *
     * \since QGIS 3.0
     */
    QgsAuxiliaryLayer *auxiliaryLayer();

    /**
     * Returns the current const auxiliary layer.
     *
     * \since QGIS 3.0
     */
    const QgsAuxiliaryLayer *auxiliaryLayer() const SIP_SKIP;

    /**
     * Read the symbology for the current layer from the Dom node supplied.
     * \param layerNode node that will contain the symbology definition for this layer.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param context reading context (used for transform from relative to absolute paths)
     * \param categories the style categories to be read
     * \returns true in case of success.
     */
    bool readSymbology( const QDomNode &layerNode, QString &errorMessage,
                        QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;

    /**
     * Read the style for the current layer from the Dom node supplied.
     * \param node node that will contain the style definition for this layer.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param context reading context (used for transform from relative to absolute paths)
     * \param categories the style categories to be read
     * \returns true in case of success.
     */
    bool readStyle( const QDomNode &node, QString &errorMessage,
                    QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) FINAL;

    /**
     * Write the symbology for the layer into the docment provided.
     *  \param node the node that will have the style element added to it.
     *  \param doc the document that will have the QDomNode added.
     *  \param errorMessage reference to string that will be updated with any error messages
     *  \param context writing context (used for transform from absolute to relative paths)
     *  \param categories the style categories to be written
     *  \returns true in case of success.
     */
    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                         const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const FINAL;

    /**
     * Write just the style information for the layer into the document
     *  \param node the node that will have the style element added to it.
     *  \param doc the document that will have the QDomNode added.
     *  \param errorMessage reference to string that will be updated with any error messages
     *  \param context writing context (used for transform from absolute to relative paths)
     *  \param categories the style categories to be written
     *  \returns true in case of success.
     */
    bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                     const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const FINAL;

    /**
     * Writes the symbology of the layer into the document provided in SLD 1.1 format
     * \param node the node that will have the style element added to it.
     * \param doc the document that will have the QDomNode added.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param props a open ended set of properties that can drive/inform the SLD encoding
     * \returns true in case of success
     */
    bool writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props = QgsStringMap() ) const;

    bool readSld( const QDomNode &node, QString &errorMessage ) FINAL;

    /**
     * Number of features rendered with specified legend key. Features must be first
     * calculated by countSymbolFeatures()
     * \returns number of features rendered by symbol or -1 if failed or counts are not available
     */
    long featureCount( const QString &legendKey ) const;

    /**
     * Determines if this vector layer has features.
     *
     * \warning when a layer is editable and some features
     * have been deleted, this will return
     * QgsFeatureSource::FeatureAvailability::FeaturesMayBeAvailable
     * to avoid a potentially expensive call to the dataprovider.
     *
     * \since QGIS 3.4
     */
    FeatureAvailability hasFeatures() const FINAL;

    /**
     * Update the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     * \param dataSource new layer data source
     * \param baseName base name of the layer
     * \param provider provider string
     * \param loadDefaultStyleFlag set to true to reset the layer's style to the default for the
     * data source
     * \since QGIS 2.10
     * \deprecated Use version with ProviderOptions argument instead
     */
    Q_DECL_DEPRECATED void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag = false ) SIP_DEPRECATED;

    /**
     * Updates the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     * \param dataSource new layer data source
     * \param baseName base name of the layer
     * \param provider provider string
     * \param options provider options
     * \param loadDefaultStyleFlag set to true to reset the layer's style to the default for the
     * data source
     * \see dataSourceChanged()
     * \since QGIS 3.2
     */
    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag = false );

    QString loadDefaultStyle( bool &resultFlag SIP_OUT ) FINAL;

    /**
     * Count features for symbols.
     * The method will return the feature counter task. You will need to
     * connect to the symbolFeatureCountMapChanged() signal to be
     * notified when the freshly updated feature counts are ready.
     *
     * \note If the count features for symbols has been already done a
     *       nullptr is returned. If you need to wait for the results,
     *       you can call waitForFinished() on the feature counter.
     *
     * \since This is asynchronous since QGIS 3.0
     */
    QgsVectorLayerFeatureCounter *countSymbolFeatures();

    /**
     * Set the string (typically sql) used to define a subset of the layer
     * \param subset The subset string. This may be the where clause of a sql statement
     *               or other definition string specific to the underlying dataprovider
     *               and data store.
     * \returns true, when setting the subset string was successful, false otherwise
     */
    virtual bool setSubsetString( const QString &subset );

    /**
     * Returns the string (typically sql) used to define a subset of the layer.
     * \returns The subset string or null QString if not implemented by the provider
     */
    virtual QString subsetString() const;

    /**
     * Query the layer for features specified in request.
     * \param request feature request describing parameters of features to return
     * \returns iterator for matching features from provider
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const FINAL;

    /**
     * Query the layer for features matching a given expression.
     */
    inline QgsFeatureIterator getFeatures( const QString &expression )
    {
      return getFeatures( QgsFeatureRequest( expression ) );
    }

    /**
     * Query the layer for the feature with the given id.
     * If there is no such feature, the returned feature will be invalid.
     */
    inline QgsFeature getFeature( QgsFeatureId fid ) const
    {
      QgsFeature feature;
      getFeatures( QgsFeatureRequest( fid ) ).nextFeature( feature );
      return feature;
    }

    /**
     * Query the layer for the geometry at the given id.
     * If there is no such feature, the returned geometry will be invalid.
     */
    QgsGeometry getGeometry( QgsFeatureId fid ) const;

    /**
     * Query the layer for the features with the given ids.
     */
    inline QgsFeatureIterator getFeatures( const QgsFeatureIds &fids )
    {
      return getFeatures( QgsFeatureRequest( fids ) );
    }

    /**
     * Query the layer for the features which intersect the specified rectangle.
     */
    inline QgsFeatureIterator getFeatures( const QgsRectangle &rectangle )
    {
      return getFeatures( QgsFeatureRequest( rectangle ) );
    }

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr ) FINAL;

    /**
     * Updates an existing \a feature in the layer, replacing the attributes and geometry for the feature
     * with matching QgsFeature::id() with the attributes and geometry from \a feature.
     * Changes are not immediately committed to the layer.
     *
     * If \a skipDefaultValue is set to true, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * Returns true if the feature's attribute was successfully changed.
     *
     * \note Calls to updateFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \warning This method needs to query the underlying data provider to fetch the feature
     * with matching QgsFeature::id() on every call. Depending on the underlying data source this
     * can be slow to execute. Consider using the more efficient changeAttributeValue() or
     * changeGeometry() methods instead.
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see changeAttributeValue()
    */
    bool updateFeature( QgsFeature &feature, bool skipDefaultValues = false );

    /**
     * Insert a new vertex before the given vertex number,
     *  in the given ring, item (first number is index 0), and feature
     *  Not meaningful for Point geometries
     *
     * \note Calls to insertVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Insert a new vertex before the given vertex number,
     *  in the given ring, item (first number is index 0), and feature
     *  Not meaningful for Point geometries
     *
     * \note Calls to insertVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Moves the vertex at the given position number,
     *  ring and item (first number is index 0), and feature
     *  to the given coordinates
     *
     * \note Calls to moveVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    /**
     * Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates
     * \note available in Python as moveVertexV2
     * \note Calls to moveVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex ) SIP_PYNAME( moveVertexV2 );

    /**
     * Deletes a vertex from a feature.
     * \param featureId ID of feature to remove vertex from
     * \param vertex index of vertex to delete
     * \note Calls to deleteVertex() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since QGIS 2.14
     */
    EditResult deleteVertex( QgsFeatureId featureId, int vertex );

    /**
     * Deletes the selected features
     *  \returns true in case of success and false otherwise
     */
    bool deleteSelectedFeatures( int *deletedCount = nullptr );

    /**
     * Adds a ring to polygon/multipolygon features
     * \param ring ring to add
     * \param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * \returns QgsGeometry::OperationResult
     * - Success
     * - LayerNotEditable
     * - AddRingNotInExistingFeature
     * - InvalidInputGeometryType
     * - AddRingNotClosed
     * - AddRingNotValid
     * - AddRingCrossesExistingRings
     *
     * \note Calls to addRing() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult addRing( const QVector<QgsPointXY> &ring, QgsFeatureId *featureId = nullptr );

    /**
     * Adds a ring to polygon/multipolygon features (takes ownership)
     * \param ring ring to add
     * \param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * \returns QgsGeometry::OperationResult
     * - Success
     * - LayerNotEditable
     * - AddRingNotInExistingFeature
     * - InvalidInputGeometryType
     * - AddRingNotClosed
     * - AddRingNotValid
     * - AddRingCrossesExistingRings
     * \note available in Python as addCurvedRing
     * \note Calls to addRing() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult addRing( QgsCurve *ring SIP_TRANSFER, QgsFeatureId *featureId = nullptr ) SIP_PYNAME( addCurvedRing );

    /**
     * Adds a new part polygon to a multipart feature
     * \returns QgsGeometry::OperationResult
     * - Success
     * - LayerNotEditable
     * - SelectionIsEmpty
     * - SelectionIsGreaterThanOne
     * - AddPartSelectedGeometryNotFound
     * - AddPartNotMultiGeometry
     * - InvalidBaseGeometry
     * - InvalidInputGeometryType
     *
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult addPart( const QList<QgsPointXY> &ring );

    /**
     * Adds a new part polygon to a multipart feature
     * \returns QgsGeometry::OperationResult
     * - Success
     * - LayerNotEditable
     * - SelectionIsEmpty
     * - SelectionIsGreaterThanOne
     * - AddPartSelectedGeometryNotFound
     * - AddPartNotMultiGeometry
     * - InvalidBaseGeometry
     * - InvalidInputGeometryType
     * \note available in Python bindings as addPartV2
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult addPart( const QgsPointSequence &ring ) SIP_PYNAME( addPartV2 );

    /**
     * \note available in Python as addCurvedPart
     * \note Calls to addPart() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult addPart( QgsCurve *ring SIP_TRANSFER ) SIP_PYNAME( addCurvedPart );

    /**
     * Translates feature by dx, dy
     *  \param featureId id of the feature to translate
     *  \param dx translation of x-coordinate
     *  \param dy translation of y-coordinate
     *  \returns 0 in case of success
     * \note Calls to translateFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    int translateFeature( QgsFeatureId featureId, double dx, double dy );

    /**
     * Splits parts cut by the given line
     *  \param splitLine line that splits the layer features
     *  \param topologicalEditing true if topological editing is enabled
     *  \returns QgsGeometry::OperationResult
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     * \note Calls to splitParts() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );

    /**
     * Splits features cut by the given line
     *  \param splitLine line that splits the layer features
     *  \param topologicalEditing true if topological editing is enabled
     *  \returns QgsGeometry::OperationResult
     * - Success
     * - NothingHappened
     * - LayerNotEditable
     * - InvalidInputGeometryType
     * - InvalidBaseGeometry
     * - GeometryEngineError
     * - SplitCannotSplitPoint
     * \note Calls to splitFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    QgsGeometry::OperationResult splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );

    /**
     * Adds topological points for every vertex of the geometry.
     * \param geom the geometry where each vertex is added to segments of other features
     * \returns 0 in case of success
     * \note geom is not going to be modified by the function
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    int addTopologicalPoints( const QgsGeometry &geom );

    /**
     * Adds a vertex to segments which intersect point p but don't
     * already have a vertex there. If a feature already has a vertex at position p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * \param p position of the vertex
     * \returns 0 in case of success
     * \note Calls to addTopologicalPoints() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    int addTopologicalPoints( const QgsPointXY &p );

    /**
     * Access to const labeling configuration. May be null if labeling is not used.
     * \note Labels will only be rendered if labelsEnabled() returns true.
     * \see labelsEnabled()
     * \since QGIS 3.0
     */
    const QgsAbstractVectorLayerLabeling *labeling() const SIP_SKIP { return mLabeling; }

    /**
     * Access to labeling configuration. May be null if labeling is not used.
     * \note Labels will only be rendered if labelsEnabled() returns true.
     * \see labelsEnabled()
     * \since QGIS 3.0
     */
    QgsAbstractVectorLayerLabeling *labeling() { return mLabeling; }

    /**
     * Set labeling configuration. Takes ownership of the object.
     * \since QGIS 3.0
     */
    void setLabeling( QgsAbstractVectorLayerLabeling *labeling SIP_TRANSFER );

    //! Returns true if the provider is in editing mode
    bool isEditable() const FINAL;

    //! Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry
    bool isSpatial() const FINAL;

    //! Returns true if the provider has been modified since the last commit
    virtual bool isModified() const;

    /**
     * Returns true if the field comes from the auxiliary layer,
     * false otherwise.
     *
     * \since QGIS 3.0
     */
    bool isAuxiliaryField( int index, int &srcIndex ) const;

    //! Synchronises with changes in the datasource
    void reload() FINAL;

    /**
     * Returns new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * \since QGIS 2.4
     */
    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) FINAL SIP_FACTORY;

    QgsRectangle extent() const FINAL;
    QgsRectangle sourceExtent() const FINAL;

    /**
     * Returns the list of fields of this layer.
     * This also includes fields which have not yet been saved to the provider.
     *
     * \returns A list of fields
     */
    QgsFields fields() const FINAL;

    /**
     * Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
     */
    inline QgsAttributeList attributeList() const { return mFields.allAttributesList(); }

    /**
     * Returns the list of attributes which make up the layer's primary keys.
     */
    QgsAttributeList primaryKeyAttributes() const;

    /**
     * Returns feature count including changes which have not yet been committed
     * If you need only the count of committed features call this method on this layer's provider.
     */
    long featureCount() const FINAL;

    /**
     * Make layer read-only (editing disabled) or not
     * \returns false if the layer is in editing yet
     */
    bool setReadOnly( bool readonly = true );

    /**
     * Changes a feature's \a geometry within the layer's edit buffer
     * (but does not immediately commit the changes). The \a fid argument
     * specifies the ID of the feature to be changed.
     *
     * If \a skipDefaultValue is set to true, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * Returns true if the feature's geometry was successfully changed.
     *
     * \note Calls to changeGeometry() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeAttributeValue()
     * \see updateFeature()
     */
    bool changeGeometry( QgsFeatureId fid, QgsGeometry &geometry, bool skipDefaultValue = false );

    /**
     * Changes an attribute value for a feature (but does not immediately commit the changes).
     * The \a fid argument specifies the ID of the feature to be changed.
     *
     * The \a field argument must specify a valid field index for the layer (where an index of 0
     * corresponds to the first field).
     *
     * The new value to be assigned to the field is given by \a newValue.
     *
     * If a valid QVariant is specified for \a oldValue, it will be used as the field value in the
     * case of an undo operation corresponding to this attribute value change. If an invalid
     * QVariant is used (the default behavior), then the feature's current value will be automatically
     * retrieved and used. Note that this involves a feature request to the underlying data provider,
     * so it is more efficient to explicitly pass an \a oldValue if it is already available.
     *
     * If \a skipDefaultValues is set to true, default field values will not
     * be updated. This can be used to override default field value expressions.
     *
     * Returns true if the feature's attribute was successfully changed.
     *
     * \note Calls to changeAttributeValue() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see updateFeature()
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant(), bool skipDefaultValues = false );

    /**
     * Changes attributes' values for a feature (but does not immediately
     * commit the changes).
     * The \a fid argument specifies the ID of the feature to be changed.
     *
     * The new values to be assigned to the fields are given by \a newValues.
     *
     * If a valid QVariant is specified for a field in \a oldValues, it will be
     * used as the field value in the case of an undo operation corresponding
     * to this attribute value change. If an invalid QVariant is used (the
     * default behavior), then the feature's current value will be
     * automatically retrieved and used. Note that this involves a feature
     * request to the underlying data provider, so it is more efficient to
     * explicitly pass an oldValue if it is already available.
     *
     * If \a skipDefaultValues is set to true, default field values will not
     * be updated. This can be used to override default field value
     * expressions.
     *
     * Returns true if feature's attributes was successfully changed.
     *
     * \note Calls to changeAttributeValues() are only valid for layers in
     * which edits have been enabled by a call to startEditing(). Changes made
     * to features using this method are not committed to the underlying data
     * provider until a commitChanges() call is made. Any uncommitted changes
     * can be discarded by calling rollBack().
     *
     * \see startEditing()
     * \see commitChanges()
     * \see changeGeometry()
     * \see updateFeature()
     * \see changeAttributeValue()
     *
     * \since QGIS 3.0
     */
    bool changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues = QgsAttributeMap(), bool skipDefaultValues = false );

    /**
     * Add an attribute field (but does not commit it)
     * returns true if the field was added
     *
     * \note Calls to addAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool addAttribute( const QgsField &field );

    /**
     * Sets an alias (a display name) for attributes to display in dialogs
     *
     * \since QGIS 3.0
     */
    void setFieldAlias( int index, const QString &aliasString );

    /**
     * Removes an alias (a display name) for attributes to display in dialogs
     *
     * \since QGIS 3.0
     */
    void removeFieldAlias( int index );

    /**
     * Renames an attribute field  (but does not commit it).
     * \param index attribute index
     * \param newName new name of field
     * \note Calls to renameAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     * \since QGIS 2.16
     */
    bool renameAttribute( int index, const QString &newName );

    /**
     * Returns the alias of an attribute name or a null string if there is no alias.
     *
     * \see {attributeDisplayName( int attributeIndex )} which returns the field name
     *      if no alias is defined.
     */
    QString attributeAlias( int index ) const;

    //! Convenience function that returns the attribute alias if defined or the field name else
    QString attributeDisplayName( int index ) const;

    //! Returns a map of field name to attribute alias
    QgsStringMap attributeAliases() const;

    /**
     * A set of attributes that are not advertised in WMS requests with QGIS server.
     */
    QSet<QString> excludeAttributesWms() const { return mExcludeAttributesWMS; }

    /**
     * A set of attributes that are not advertised in WMS requests with QGIS server.
     */
    void setExcludeAttributesWms( const QSet<QString> &att ) { mExcludeAttributesWMS = att; }

    /**
     * A set of attributes that are not advertised in WFS requests with QGIS server.
     */
    QSet<QString> excludeAttributesWfs() const { return mExcludeAttributesWFS; }

    /**
     * A set of attributes that are not advertised in WFS requests with QGIS server.
     */
    void setExcludeAttributesWfs( const QSet<QString> &att ) { mExcludeAttributesWFS = att; }

    /**
     * Deletes an attribute field (but does not commit it).
     *
     * \note Calls to deleteAttribute() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    virtual bool deleteAttribute( int attr );

    /**
     * Deletes a list of attribute fields (but does not commit it)
     *
     * \param  attrs the indices of the attributes to delete
     * \returns true if at least one attribute has been deleted
     *
     */
    bool deleteAttributes( const QList<int> &attrs );

    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) FINAL;

    /**
     * Deletes a feature from the layer (but does not commit it).
     *
     * \note Calls to deleteFeature() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool deleteFeature( QgsFeatureId fid );

    /**
     * Deletes a set of features from the layer (but does not commit it)
     * \param fids The feature ids to delete
     *
     * \returns false if the layer is not in edit mode or does not support deleting
     *         in case of an active transaction depends on the provider implementation
     *
     * \note Calls to deleteFeatures() are only valid for layers in which edits have been enabled
     * by a call to startEditing(). Changes made to features using this method are not committed
     * to the underlying data provider until a commitChanges() call is made. Any uncommitted
     * changes can be discarded by calling rollBack().
     */
    bool deleteFeatures( const QgsFeatureIds &fids );

    /**
     * Attempts to commit to the underlying data provider any buffered changes made since the
     * last to call to startEditing().
     *
     * Returns the result of the attempt. If a commit fails (i.e. false is returned), the
     * in-memory changes are left untouched and are not discarded. This allows editing to
     * continue if the commit failed on e.g. a disallowed value in a Postgres
     * database - the user can re-edit and try again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it can be difficult to roll back cleanly.
     * Therefore any error message returned by commitErrors() also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
     *
     * \see startEditing()
     * \see commitErrors()
     * \see rollBack()
     */
    bool commitChanges();

    /**
     * Returns a list containing any error messages generated when attempting
     * to commit changes to the layer.
     * \see commitChanges()
     */
    QStringList commitErrors() const;

    /**
     * Stops a current editing operation and discards any uncommitted edits.
     *
     * If \a deleteBuffer is true the editing buffer will be completely deleted (the default
     * behavior).
     *
     * \see startEditing()
     * \see commitChanges()
     */
    bool rollBack( bool deleteBuffer = true );

    /**
     * Returns the layer's relations, where the foreign key is on this layer.
     *
     * \param idx Only get relations, where idx forms part of the foreign key
     * \returns A list of relations
     */
    QList<QgsRelation> referencingRelations( int idx ) const;

    //! Buffer with uncommitted editing operations. Only valid after editing has been turned on.
    QgsVectorLayerEditBuffer *editBuffer() { return mEditBuffer; }

    /**
     * Buffer with uncommitted editing operations. Only valid after editing has been turned on.
     * \note not available in Python bindings
     */
    const QgsVectorLayerEditBuffer *editBuffer() const SIP_SKIP { return mEditBuffer; }

    /**
     * Create edit command for undo/redo operations
     * \param text text which is to be displayed in undo window
     */
    void beginEditCommand( const QString &text );

    //! Finish edit command and add it to undo/redo stack
    void endEditCommand();

    //! Destroy active command and reverts all changes in it
    void destroyEditCommand();

    //! Editing vertex markers
    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker
    };

    /**
     * Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.)
     * \deprecated Use the equivalent QgsSymbolLayerUtils::drawVertexMarker function instead
     */
    Q_DECL_DEPRECATED static void drawVertexMarker( double x, double y, QPainter &p, QgsVectorLayer::VertexMarkerType type, int vertexSize );

    /**
     * Will regenerate the `fields` property of this layer by obtaining all fields
     * from the dataProvider, joined fields and virtual fields. It will also
     * take any changes made to default values into consideration.
     *
     * \note Unless the fields on the provider have directly been modified, there is
     * no reason to call this method.
     */
    void updateFields();

    /**
     * Returns the calculated default value for the specified field index. The default
     * value may be taken from a client side default value expression (see setDefaultValueDefinition())
     * or taken from the underlying data provider.
     * \param index field index
     * \param feature optional feature to use for default value evaluation. If passed,
     * then properties from the feature (such as geometry) can be used when calculating
     * the default value.
     * \param context optional expression context to evaluate expressions again. If not
     * specified, a default context will be created
     * \returns calculated default value
     * \see setDefaultValueDefinition()
     * \since QGIS 3.0
     */
    QVariant defaultValue( int index, const QgsFeature &feature = QgsFeature(),
                           QgsExpressionContext *context = nullptr ) const;

    /**
     * Sets the definition of the expression to use when calculating the default value for a field.
     * \param index field index
     * \param definition default value definition to use and evaluate
     * when calculating default values for field. Pass
     * an empty expression to clear the default.
     *
     * \see defaultValue()
     * \see defaultValueDefinition()
     * \since QGIS 3.0
     */
    void setDefaultValueDefinition( int index, const QgsDefaultValue &definition );

    /**
     * Returns the definition of the expression used when calculating the default value for a field.
     * \param index field index
     * \returns definition of the default value with the expression evaluated
     * when calculating default values for field, or definition with an
     * empty string if no default is set
     * \see defaultValue()
     * \see setDefaultValueDefinition()
     * \since QGIS 3.0
     */
    QgsDefaultValue defaultValueDefinition( int index ) const;

    /**
     * Returns any constraints which are present for a specified
     * field index. These constraints may be inherited from the layer's data provider
     * or may be set manually on the vector layer from within QGIS.
     * \see setFieldConstraint()
     * \since QGIS 3.0
     */
    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;

    /**
     * Returns a map of constraint with their strength for a specific field of the layer.
     * \param fieldIndex field index
     * \since QGIS 3.0
     */
    QMap< QgsFieldConstraints::Constraint, QgsFieldConstraints::ConstraintStrength> fieldConstraintsAndStrength( int fieldIndex ) const;

    /**
     * Sets a constraint for a specified field index. Any constraints inherited from the layer's
     * data provider will be kept intact and cannot be modified. Ie, calling this method only allows for new
     * constraints to be added on top of the existing provider constraints.
     * \see fieldConstraints()
     * \see removeFieldConstraint()
     * \since QGIS 3.0
     */
    void setFieldConstraint( int index, QgsFieldConstraints::Constraint constraint, QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthHard );

    /**
     * Removes a constraint for a specified field index. Any constraints inherited from the layer's
     * data provider will be kept intact and cannot be removed.
     * \see fieldConstraints()
     * \see setFieldConstraint()
     * \since QGIS 3.0
     */
    void removeFieldConstraint( int index, QgsFieldConstraints::Constraint constraint );

    /**
     * Returns the constraint expression for for a specified field index, if set.
     * \see fieldConstraints()
     * \see constraintDescription()
     * \see setConstraintExpression()
     * \since QGIS 3.0
     */
    QString constraintExpression( int index ) const;

    /**
     * Returns the descriptive name for the constraint expression for a specified field index.
     * \see fieldConstraints()
     * \see constraintExpression()
     * \see setConstraintExpression()
     * \since QGIS 3.0
     */
    QString constraintDescription( int index ) const;

    /**
     * Set the constraint expression for the specified field index. An optional descriptive name for the constraint
     * can also be set. Setting an empty expression will clear any existing expression constraint.
     * \see constraintExpression()
     * \see constraintDescription()
     * \see fieldConstraints()
     * \since QGIS 3.0
     */
    void setConstraintExpression( int index, const QString &expression, const QString &description = QString() );

    /**
     * \copydoc editorWidgetSetup
     */
    void setEditorWidgetSetup( int index, const QgsEditorWidgetSetup &setup );

    /**
     * The editor widget setup defines which QgsFieldFormatter and editor widget will be used
     * for the field at `index`.
     *
     * \since QGIS 3.0
     */
    QgsEditorWidgetSetup editorWidgetSetup( int index ) const;

    /**
     * Calculates a list of unique values contained within an attribute in the layer. Note that
     * in some circumstances when unsaved changes are present for the layer then the returned list
     * may contain outdated values (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value will be included in the
     * returned list).
     * \param fieldIndex column index for attribute
     * \param limit maximum number of values to return (or -1 if unlimited)
     * \see minimumValue()
     * \see maximumValue()
     */
    QSet<QVariant> uniqueValues( int fieldIndex, int limit = -1 ) const FINAL;

    /**
     * Returns unique string values of an attribute which contain a specified subset string. Subset
     * matching is done in a case-insensitive manner. Note that
     * in some circumstances when unsaved changes are present for the layer then the returned list
     * may contain outdated values (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value will be included in the
     * returned list).
     * \param index column index for attribute
     * \param substring substring to match (case insensitive)
     * \param limit maxmum number of the values to return, or -1 to return all unique values
     * \param feedback optional feedback object for canceling request
     * \returns list of unique strings containing substring
     */
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const;

    /**
     * Returns the minimum value for an attribute column or an invalid variant in case of error.
     * Note that in some circumstances when unsaved changes are present for the layer then the
     * returned value may be outdated (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value may be returned as the minimum).
     * \see maximumValue()
     * \see uniqueValues()
     */
    QVariant minimumValue( int index ) const FINAL;

    /**
     * Returns the maximum value for an attribute column or an invalid variant in case of error.
     * Note that in some circumstances when unsaved changes are present for the layer then the
     * returned value may be outdated (for instance when the attribute value in a saved feature has
     * been changed inside the edit buffer then the previous saved value may be returned as the maximum).
     * \see minimumValue()
     * \see uniqueValues()
     */
    QVariant maximumValue( int index ) const FINAL;

    /**
     * Calculates an aggregated value from the layer's features.
     * \param aggregate aggregate to calculate
     * \param fieldOrExpression source field or expression to use as basis for aggregated values.
     * \param parameters parameters controlling aggregate calculation
     * \param context expression context for expressions and filters
     * \param ok if specified, will be set to true if aggregate calculation was successful
     * \returns calculated aggregate value
     * \since QGIS 2.16
     */
    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString &fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters &parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext *context = nullptr,
                        bool *ok = nullptr ) const;

    //! Sets the blending mode used for rendering each feature
    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
    //! Returns the current blending mode for features
    QPainter::CompositionMode featureBlendMode() const;

    /**
     * Sets the \a opacity for the vector layer, where \a opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see opacity()
     * \see opacityChanged()
     * \since QGIS 3.0
     */
    void setOpacity( double opacity );

    /**
     * Returns the opacity for the vector layer, where opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see setOpacity()
     * \see opacityChanged()
     * \since QGIS 3.0
     */
    double opacity() const;

    QString htmlMetadata() const FINAL;

    /**
     * Set the simplification settings for fast rendering of features
     *  \since QGIS 2.2
     */
    void setSimplifyMethod( const QgsVectorSimplifyMethod &simplifyMethod ) { mSimplifyMethod = simplifyMethod; }

    /**
     * Returns the simplification settings for fast rendering of features
     *  \since QGIS 2.2
     */
    inline const QgsVectorSimplifyMethod &simplifyMethod() const { return mSimplifyMethod; }

    /**
     * Returns whether the VectorLayer can apply the specified simplification hint
     *  \note Do not use in 3rd party code - may be removed in future version!
     *  \since QGIS 2.2
     */
    bool simplifyDrawingCanbeApplied( const QgsRenderContext &renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;

    /**
     * Returns the conditional styles that are set for this layer. Style information is
     * used to render conditional formatting in the attribute table.
     * \returns Return a QgsConditionalLayerStyles object holding the conditional attribute
     * style information. Style information is generic and can be used for anything.
     * \since QGIS 2.12
     */
    QgsConditionalLayerStyles *conditionalStyles() const;

    /**
     * Returns the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    QgsAttributeTableConfig attributeTableConfig() const;

    /**
     * Set the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig &attributeTableConfig );

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.0
     */
    QString mapTipTemplate() const;

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.0
     */
    void setMapTipTemplate( const QString &mapTipTemplate );

    QgsExpressionContext createExpressionContext() const FINAL;

    QgsExpressionContextScope *createExpressionContextScope() const FINAL SIP_FACTORY;

    /**
     * Returns the configuration of the form used to represent this vector layer.
     *
     * \returns The configuration of this layers' form
     *
     * \since QGIS 2.14
     */
    QgsEditFormConfig editFormConfig() const;

    /**
     * Set the \a editFormConfig (configuration) of the form used to represent this vector layer.
     *
     * \see editFormConfig()
     * \since QGIS 3.0
     */
    void setEditFormConfig( const QgsEditFormConfig &editFormConfig );

    /**
     * Flag allowing to indicate if the extent has to be read from the XML
     * document when data source has no metadata or if the data provider has
     * to determine it.
     *
     * \since QGIS 3.0
     */
    void setReadExtentFromXml( bool readExtentFromXml );

    /**
     * Returns true if the extent is read from the XML document when data
     * source has no metadata, false if it's the data provider which determines
     * it.
     *
     * \since QGIS 3.0
     */
    bool readExtentFromXml() const;

    /**
     * Test if an edit command is active
     *
     * \since QGIS 3.0
     */
    bool isEditCommandActive() const { return mEditCommandActive; }

    /**
     * Configuration and logic to apply automatically on any edit happening on this layer.
     *
     * \since QGIS 3.4
     */
    QgsGeometryOptions *geometryOptions() const;

    /**
     * Controls, if the layer is allowed to commit changes. If this is set to false
     * it will not be possible to commit changes on this layer. This can be used to
     * define checks on a layer that need to be pass before the layer can be saved.
     * If you use this API, make sure that:
     *
     *  - the user is visibly informed that his changes were not saved and what he needs
     *    to do in order to be able to save the changes.
     *
     *  - to set the property back to true, once the user has fixed his data.
     *
     * When calling \see commitChanges(), this flag is checked just after the
     * \see beforeCommitChanges() signal is emitted, so it's possible to adjust it from there.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.4
     */
    bool allowCommit() const SIP_SKIP;

    /**
     * Controls, if the layer is allowed to commit changes. If this is set to false
     * it will not be possible to commit changes on this layer. This can be used to
     * define checks on a layer that need to be pass before the layer can be saved.
     * If you use this API, make sure that:
     *
     *  - the user is visibly informed that his changes were not saved and what he needs
     *    to do in order to be able to save the changes.
     *
     *  - to set the property back to true, once the user has fixed his data.
     *
     * When calling \see commitChanges(), this flag is checked just after the
     * \see beforeCommitChanges() signal is emitted, so it's possible to adjust it from there.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.4
     */
    void setAllowCommit( bool allowCommit ) SIP_SKIP;

  public slots:

    /**
     * Select feature by its ID
     *
     * \param featureId  The id of the feature to select
     *
     * \see select( const QgsFeatureIds& )
     */
    void select( QgsFeatureId featureId );

    /**
     * Select features by their ID
     *
     * \param featureIds The ids of the features to select
     *
     * \see select(QgsFeatureId)
     */
    void select( const QgsFeatureIds &featureIds );

    /**
     * Deselect feature by its ID
     *
     * \param featureId  The id of the feature to deselect
     *
     * \see deselect(const QgsFeatureIds&)
     */
    void deselect( QgsFeatureId featureId );

    /**
     * Deselect features by their ID
     *
     * \param featureIds The ids of the features to deselect
     *
     * \see deselect(const QgsFeatureId)
     */
    void deselect( const QgsFeatureIds &featureIds );

    /**
     * Clear selection
     *
     * \see selectByIds()
     */
    void removeSelection();

    /**
     * Update the extents for the layer. This is necessary if features are
     *  added/deleted or the layer has been subsetted.
     *
     * \param force true to update layer extent even if it's read from xml by default, false otherwise
     */
    virtual void updateExtents( bool force = false );

    /**
     * Makes the layer editable.
     *
     * This starts an edit session on this layer. Changes made in this edit session will not
     * be made persistent until commitChanges() is called, and can be reverted by calling
     * rollBack().
     *
     * Returns true if the layer was successfully made editable, or false if the operation
     * failed (e.g. due to an underlying read-only data source, or lack of edit support
     * by the backend data provider).
     *
     * \see commitChanges()
     * \see rollBack()
     */
    bool startEditing();

  signals:

    /**
     * Emitted whenever the layer's data source has been changed.
     *
     * \see setDataSource()
     *
     * \since QGIS 3.4
     */
    void dataSourceChanged();

    /**
     * This signal is emitted when selection was changed
     *
     * \param selected        Newly selected feature ids
     * \param deselected      Ids of all features which have previously been selected but are not any more
     * \param clearAndSelect  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect );

    //! This signal is emitted when modifications has been done on layer
    void layerModified();

    /**
     * Emitted whenever the allowCommitChanged() property of this layer changes.
     *
     * \since QGIS 3.4
     */
    void allowCommitChanged();

    //! Is emitted, when layer is checked for modifications. Use for last-minute additions
    void beforeModifiedCheck() const;

    //! Is emitted, before editing on this layer is started
    void beforeEditingStarted();

    //! Is emitted, when editing on this layer has started
    void editingStarted();

    //! Is emitted, when edited changes successfully have been written to the data provider
    void editingStopped();

    //! Is emitted, before changes are committed to the data provider
    void beforeCommitChanges();

    //! Is emitted, before changes are rolled back
    void beforeRollBack();

    /**
     * Is emitted, after changes are rolled back
     * \since QGIS 3.4
     */
    void afterRollBack();

    /**
     * Will be emitted, when a new attribute has been added to this vector layer.
     * Applies only to types QgsFields::OriginEdit, QgsFields::OriginProvider and QgsFields::OriginExpression
     *
     * \param idx The index of the new attribute
     *
     * \see updatedFields()
     */
    void attributeAdded( int idx );

    /**
     * Will be emitted, when an expression field is going to be added to this vector layer.
     * Applies only to types QgsFields::OriginExpression
     *
     * \param fieldName The name of the attribute to be added
     */
    void beforeAddingExpressionField( const QString &fieldName );

    /**
     * Will be emitted, when an attribute has been deleted from this vector layer.
     * Applies only to types QgsFields::OriginEdit, QgsFields::OriginProvider and QgsFields::OriginExpression
     *
     * \param idx The index of the deleted attribute
     *
     * \see updatedFields()
     */
    void attributeDeleted( int idx );

    /**
     * Will be emitted, when an expression field is going to be deleted from this vector layer.
     * Applies only to types QgsFields::OriginExpression
     *
     * \param idx The index of the attribute to be deleted
     */
    void beforeRemovingExpressionField( int idx );

    /**
     * Emitted when a new feature has been added to the layer
     *
     * \param fid The id of the new feature
     */
    void featureAdded( QgsFeatureId fid );

    /**
     * Emitted when a feature has been deleted.
     *
     * If you do expensive operations in a slot connected to this, you should prefer to use
     * featuresDeleted( const QgsFeatureIds& ).
     *
     * \param fid The id of the feature which has been deleted
     */
    void featureDeleted( QgsFeatureId fid );

    /**
     * Emitted when features have been deleted.
     *
     * If features are deleted within an edit command, this will only be emitted once at the end
     * to allow connected slots to minimize the overhead.
     * If features are deleted outside of an edit command, this signal will be emitted once per feature.
     *
     * \param fids The feature ids that have been deleted.
     */
    void featuresDeleted( const QgsFeatureIds &fids );

    /**
     * Is emitted, whenever the fields available from this layer have been changed.
     * This can be due to manually adding attributes or due to a join.
     */
    void updatedFields();

    /**
     * Emitted when the layer's subset string has changed.
     * \since QGIS 3.2
     */
    void subsetStringChanged();

    /**
     * Is emitted whenever an attribute value change is done in the edit buffer.
     * Note that at this point the attribute change is not yet saved to the provider.
     *
     * \param fid The id of the changed feature
     * \param idx The attribute index of the changed attribute
     * \param value The new value of the attribute
     */
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );

    /**
     * Is emitted whenever a geometry change is done in the edit buffer.
     * Note that at this point the geometry change is not yet saved to the provider.
     *
     * \param fid The id of the changed feature
     * \param geometry The new geometry
     */
    void geometryChanged( QgsFeatureId fid, const QgsGeometry &geometry );

    //! This signal is emitted, when attributes are deleted from the provider
    void committedAttributesDeleted( const QString &layerId, const QgsAttributeList &deletedAttributes );
    //! This signal is emitted, when attributes are added to the provider
    void committedAttributesAdded( const QString &layerId, const QList<QgsField> &addedAttributes );
    //! This signal is emitted, when features are added to the provider
    void committedFeaturesAdded( const QString &layerId, const QgsFeatureList &addedFeatures );
    //! This signal is emitted, when features are deleted from the provider
    void committedFeaturesRemoved( const QString &layerId, const QgsFeatureIds &deletedFeatureIds );
    //! This signal is emitted, when attribute value changes are saved to the provider
    void committedAttributeValuesChanges( const QString &layerId, const QgsChangedAttributesMap &changedAttributesValues );
    //! This signal is emitted, when geometry changes are saved to the provider
    void committedGeometriesChanges( const QString &layerId, const QgsGeometryMap &changedGeometries );

    //! Emitted when the font family defined for labeling layer is not found on system
    void labelingFontNotFound( QgsVectorLayer *layer, const QString &fontfamily );

    //! Signal emitted when setFeatureBlendMode() is called
    void featureBlendModeChanged( QPainter::CompositionMode blendMode );

    /**
     * Emitted when the layer's opacity is changed, where \a opacity is a value between 0 (transparent)
     * and 1 (opaque).
     * \see setOpacity()
     * \see opacity()
     * \since QGIS 3.0
     */
    void opacityChanged( double opacity );

    /**
     * Signal emitted when a new edit command has been started
     *
     * \param text Description for this edit command
     */
    void editCommandStarted( const QString &text );

    /**
     * Signal emitted, when an edit command successfully ended
     * \note This does not mean it is also committed, only that it is written
     * to the edit buffer. See beforeCommitChanges()
     */
    void editCommandEnded();

    /**
     * Signal emitted, when an edit command is destroyed
     * \note This is not a rollback, it is only related to the current edit command.
     * See beforeRollBack()
     */
    void editCommandDestroyed();

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being read.
     * If there is custom style information saved in the file, you can connect to this signal
     * and update the layer style accordingly.
     *
     * \param element The XML layer style element.
     *
     * \param errorMessage Write error messages into this string.
     */
    void readCustomSymbology( const QDomElement &element, QString &errorMessage );

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being written.
     * If there is custom style information you want to save to the file, you can connect
     * to this signal and update the element accordingly.
     *
     * \param element  The XML element where you can add additional style information to.
     * \param doc      The XML document that you can use to create new XML nodes.
     * \param errorMessage Write error messages into this string.
     */
    void writeCustomSymbology( QDomElement &element, QDomDocument &doc, QString &errorMessage ) const;

    /**
     * Emitted when the map tip changes
     *
     * \since QGIS 3.0
     */
    void mapTipTemplateChanged();

    /**
     * Emitted when the display expression changes
     *
     * \since QGIS 3.0
     */
    void displayExpressionChanged();

    /**
     * Signals an error related to this vector layer.
     */
    void raiseError( const QString &msg );

    /**
     * Will be emitted whenever the edit form configuration of this layer changes.
     *
     * \since QGIS 3.0
     */
    void editFormConfigChanged();

    /**
     * Emitted when the read only state of this layer is changed.
     * Only applies to manually set readonly state, not to the edit mode.
     *
     * \since QGIS 3.0
     */
    void readOnlyChanged();

    /**
     * Emitted when the feature count for symbols on this layer has been recalculated.
     *
     * \since QGIS 3.0
     */
    void symbolFeatureCountMapChanged();

  protected:
    //! Sets the extent
    void setExtent( const QgsRectangle &rect ) FINAL;

  private slots:
    void invalidateSymbolCountedFlag();
    void onFeatureCounterCompleted();
    void onFeatureCounterTerminated();
    void onJoinedFieldsChanged();
    void onFeatureDeleted( QgsFeatureId fid );
    void onRelationsLoaded();
    void onSymbolsCounted();
    void onDirtyTransaction( const QString &sql, const QString &name );

  private:
    void updateDefaultValues( QgsFeatureId fid, QgsFeature feature = QgsFeature() );

    /**
     * Returns true if the provider is in read-only mode
     */
    bool isReadOnly() const FINAL;

    /**
     * Bind layer to a specific data provider
     * \param provider provider key string, must match a valid QgsVectorDataProvider key. E.g. "postgres", "ogr", etc.
     * \param options provider options
     */
    bool setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options );

    //! Read labeling from SLD
    void readSldLabeling( const QDomNode &node );

    //! Read simple labeling from layer's custom properties (QGIS 2.x projects)
    QgsAbstractVectorLayerLabeling *readLabelingFromCustomProperties();

#ifdef SIP_RUN
    QgsVectorLayer( const QgsVectorLayer &rhs );
#endif

  private:                       // Private attributes
    QgsConditionalLayerStyles *mConditionalStyles = nullptr;

    //! Pointer to data provider derived from the abastract base class QgsDataProvider
    QgsVectorDataProvider *mDataProvider = nullptr;

    //! The preview expression used to generate a human readable preview string for features
    QString mDisplayExpression;

    QString mMapTipTemplate;

    //! Data provider key
    QString mProviderKey;

    //! The user-defined actions that are accessed from the Identify Results dialog box
    QgsActionManager *mActions = nullptr;

    //! Flag indicating whether the layer is in read-only mode (editing disabled) or not
    bool mReadOnly = false;

    /**
     * Set holding the feature IDs that are activated.  Note that if a feature
        subsequently gets deleted (i.e. by its addition to mDeletedFeatureIds),
        it always needs to be removed from mSelectedFeatureIds as well.
     */
    QgsFeatureIds mSelectedFeatureIds;

    //! Field map to commit
    QgsFields mFields;

    //! Map that stores the aliases for attributes. Key is the attribute name and value the alias for that attribute
    QgsStringMap mAttributeAliasMap;

    //! Map which stores default value expressions for fields
    QMap<QString, QgsDefaultValue> mDefaultExpressionMap;

    //! An internal structure to keep track of fields that have a defaultValueOnUpdate
    QSet<int> mDefaultValueOnUpdateFields;

    //! Map which stores constraints for fields
    QMap< QString, QgsFieldConstraints::Constraints > mFieldConstraints;

    //! Map which stores constraint strength for fields
    QMap< QPair< QString, QgsFieldConstraints::Constraint >, QgsFieldConstraints::ConstraintStrength > mFieldConstraintStrength;

    //! Map which stores expression constraints for fields. Value is a pair of expression/description.
    QMap< QString, QPair< QString, QString > > mFieldConstraintExpressions;

    QMap< QString, QgsEditorWidgetSetup > mFieldWidgetSetups;

    //! Holds the configuration for the edit form
    QgsEditFormConfig mEditFormConfig;

    //! Attributes which are not published in WMS
    QSet<QString> mExcludeAttributesWMS;

    //! Attributes which are not published in WFS
    QSet<QString> mExcludeAttributesWFS;

    //! Geometry type as defined in enum WkbType (qgis.h)
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    //! Renderer object which holds the information about how to display the features
    QgsFeatureRenderer *mRenderer = nullptr;

    //! Simplification object which holds the information about how to simplify the features for fast rendering
    QgsVectorSimplifyMethod mSimplifyMethod;

    //! Labeling configuration
    QgsAbstractVectorLayerLabeling *mLabeling = nullptr;

    //! True if labels are enabled
    bool mLabelsEnabled = false;

    //! Whether 'labeling font not found' has be shown for this layer (only show once in QgsMessageBar, on first rendering)
    bool mLabelFontNotFoundNotified = false;

    //! Blend mode for features
    QPainter::CompositionMode mFeatureBlendMode = QPainter::CompositionMode_SourceOver;

    //! Layer opacity
    double mLayerOpacity = 1.0;

    //! Flag if the vertex markers should be drawn only for selection (true) or for all features (false)
    bool mVertexMarkerOnlyForSelection = false;

    QStringList mCommitErrors;

    //! stores information about uncommitted changes to layer
    QgsVectorLayerEditBuffer *mEditBuffer = nullptr;
    friend class QgsVectorLayerEditBuffer;
    friend class QgsVectorLayerEditPassthrough;

    //stores information about joined layers
    QgsVectorLayerJoinBuffer *mJoinBuffer = nullptr;

    //! stores information about expression fields on this layer
    QgsExpressionFieldBuffer *mExpressionFieldBuffer = nullptr;

    //diagram rendering object. 0 if diagram drawing is disabled
    QgsDiagramRenderer *mDiagramRenderer = nullptr;

    //stores infos about diagram placement (placement type, priority, position distance)
    QgsDiagramLayerSettings *mDiagramLayerSettings = nullptr;

    mutable bool mValidExtent = false;
    mutable bool mLazyExtent = true;

    //! Auxiliary layer
    std::unique_ptr<QgsAuxiliaryLayer> mAuxiliaryLayer;

    //! Key to use to join auxiliary layer
    QString mAuxiliaryLayerKey;

    // Features in renderer classes counted
    bool mSymbolFeatureCounted = false;

    // Feature counts for each renderer legend key
    QHash<QString, long> mSymbolFeatureCountMap;

    //! True while an undo command is active
    bool mEditCommandActive = false;

    bool mReadExtentFromXml;
    QgsRectangle mXmlExtent;

    QgsFeatureIds mDeletedFids;

    QgsAttributeTableConfig mAttributeTableConfig;

    mutable QMutex mFeatureSourceConstructorMutex;

    QgsVectorLayerFeatureCounter *mFeatureCounter = nullptr;

    std::unique_ptr<QgsGeometryOptions> mGeometryOptions;

    bool mAllowCommit = true;

    friend class QgsVectorLayerFeatureSource;
};



// clazy:excludeall=qstring-allocations

#endif
