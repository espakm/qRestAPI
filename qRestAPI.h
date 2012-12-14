/*==============================================================================

  Program: qRestAPI

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qRestAPI_h
#define __qRestAPI_h

// Qt includes
#include <QMap>
#include <QObject>
#include <QUuid>
#include <QVariant>

template <class Key, class T> class QMap;
typedef QMap<QString, QVariant> QVariantMap;

class qRestAPIPrivate;

/// qRestAPI is a simple interface class to communicate with a Midas3 public
/// API.
/// Queries are posted to the server and answers reported back.
/// qRestAPI works in synchronous or unsynchronous way.
/// Usage:
/// <code>
/// qRestAPI midas;
/// midas.setMidasUrl("http://slicer.kitware.com/midas3");
/// connect(&midas, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
///         myApp, SLOT(processResult(QUuid,QList<QVariantMap>)));
/// midas.query("midas.version");
/// ...
/// </code>
class qRestAPI : public QObject
{
  Q_OBJECT
  /// Url of the Midas server. e.g. "http://slicer.kitware.com/midas3"
  Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl)

  /// Max time to wait until last progress of a query
  Q_PROPERTY(int timeOut READ timeOut WRITE setTimeOut)

  /// Default raw headers to be set for each requests. Can be used to set
  /// e.g. the user-agent or authentication information.
  Q_PROPERTY(RawHeadersType defaultRawHeaders READ defaultRawHeaders WRITE setDefaultRawHeaders)

  /// Suppress SSL errors
  Q_PROPERTY(bool suppressSslErrors READ suppressSslErrors WRITE setSuppressSslErrors)

public:
  typedef QObject Superclass;
  explicit qRestAPI(QObject*parent = 0);
  virtual ~qRestAPI();

  typedef QMap<QString, QString> ParametersType;
//  typedef QNetworkCacheMetaData::RawHeaderList RawHeadersType;
//  typedef QPair<QByteArray, QByteArray> RawHeaderType;
//  typedef QList<RawHeaderType> RawHeadersType;
  typedef QMap<QByteArray, QByteArray> RawHeadersType;

  QString serverUrl()const;
  void setServerUrl(const QString& newServerUrl);

  RawHeadersType defaultRawHeaders()const;
  void setDefaultRawHeaders(const RawHeadersType& defaultRawHeaders);

  bool suppressSslErrors()const;
  void setSuppressSslErrors(bool suppressSslErrors);

  /// Post a query on the Midas server. The \a method and \parameters
  /// are used to compose the query.
  /// errorReceived() is emitted if no server is found or if the server sends
  /// errors.
  /// resultReceived() is emitted when a result is received from the server,
  /// it is fired even if errors are received.
  /// Returns a unique identifiant of the posted query.
  QUuid query(const QString& method,
    const ParametersType& parameters = ParametersType(),
    const RawHeadersType& rawHeaders = RawHeadersType());

  /// Utility function that waits \a maxWaitingTimeInMSecs msecs for the result
  /// of the query. Returns the answer of the server or an empty map if the
  /// result failed.
  /// If an error is emitted, "queryError" is added to the output.
  /// Internally, a QEventLoop is used so it can have side effects on your
  /// application.
  static QList<QVariantMap> synchronousQuery(bool &ok,
    const QString& midasUrl,
    const QString& method, const ParametersType& parameters = ParametersType(),
    const RawHeadersType& rawHeaders = RawHeadersType(),
    int maxWaitingTimeInMSecs = 2500);

  /// Utility function that transforms a QList of QVariantMap into a string.
  /// Mostly for debug purpose.
  static QString qVariantMapListToString(const QList<QVariantMap>& variants);

  void setTimeOut(int msecs);
  int timeOut()const;
signals:
  void infoReceived(const QString& message);
  void errorReceived(const QString& errorMessage);
  void resultReceived(QUuid queryUuid, const QList<QVariantMap>&);

protected:
  qRestAPI(qRestAPIPrivate* d, QObject* parent = 0);

  QScopedPointer<qRestAPIPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qRestAPI);
  Q_DISABLE_COPY(qRestAPI);
};

#endif

