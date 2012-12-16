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
#include <QObject>
#include <QUuid>
#include <QVariant>

template <class Key, class T> class QMap;
typedef QMap<QString, QVariant> QVariantMap;

class qRestAPIPrivate;

/// qRestAPI is a simple interface class to communicate with web services
/// through a public RESTful API.
/// Queries are posted to the server and answers reported back.
/// qRestAPI works in synchronous or asynchronous way.
/// The class should be adopted to specific web services by subclassing. The
/// derived class should define how to construct the requests and how to
/// interpret the results.
/// The library provides a sample implementation to interact with Midas servers.
/// Usage:
/// <code>
/// qRestAPI* midas = new qMidasAPI();
/// midas->setServerUrl("http://slicer.kitware.com/midas3");
/// connect(midas, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
///         myApp, SLOT(processResult(QUuid,QList<QVariantMap>)));
/// midas->query("midas.version");
/// ...
/// delete midas;
/// </code>
class qRestAPI : public QObject
{
  Q_OBJECT

  /// Url of the web application. e.g. "http://slicer.kitware.com/midas3"
  Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl)

  /// Max time to wait until last progress of a query
  Q_PROPERTY(int timeOut READ timeOut WRITE setTimeOut)

  /// Default raw headers to be set for each requests. E.g. it can be used to set
  /// the user-agent or authentication information.
  Q_PROPERTY(RawHeaders defaultRawHeaders READ defaultRawHeaders WRITE setDefaultRawHeaders)

  /// Suppress SSL errors
  Q_PROPERTY(bool suppressSslErrors READ suppressSslErrors WRITE setSuppressSslErrors)

  typedef QObject Superclass;

public:
  explicit qRestAPI(QObject*parent = 0);
  virtual ~qRestAPI();

  typedef QMap<QString, QString> Parameters;
  typedef QMap<QByteArray, QByteArray> RawHeaders;

  QString serverUrl()const;
  void setServerUrl(const QString& newServerUrl);

  RawHeaders defaultRawHeaders()const;
  void setDefaultRawHeaders(const RawHeaders& defaultRawHeaders);

  bool suppressSslErrors()const;
  void setSuppressSslErrors(bool suppressSslErrors);

  /// Post a query to the web service. The \a resource and \parameters
  /// are used to compose the query.
  /// \a rawHeaders can be used to set the raw headers of the request to send.
  /// These headers will be set additionally to those defined by the
  /// \a defaultRawHeaders property.
  /// errorReceived() is emitted if no server is found or if the server sends
  /// errors.
  /// resultReceived() is emitted when a result is received from the server,
  /// it is fired even if errors are received.
  /// Returns a unique identifier of the posted query.
  QUuid query(const QString& resource,
    const Parameters& parameters = Parameters(),
    const RawHeaders& rawHeaders = RawHeaders());

  /// Like \a query, but blocks the current thread until the web service
  /// responds or the \a timeOut expires.
  /// Internally, a QEventLoop is used so it can have side effects on your
  /// application.
  /// On success, it returns the answer of the server and sets \a ok to
  /// \a true. Otherwise, an empty map is returned and \ok is set to \a
  /// false.
  /// If an error is emitted, "queryError" is added to the output.
  QList<QVariantMap> synchronousQuery(bool &ok,
    const QString& resource,
    const Parameters& parameters = Parameters(),
    const RawHeaders& rawHeaders = RawHeaders());

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
