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

// Qt includes
#include <QDebug>
#include <QEventLoop>
#include <QScriptValueIterator>
#include <QSslSocket>
#include <QStringList>
#include <QTimer>
#include <QUuid>
#include <QDebug>

// qRestAPI includes
#include "qRestAPI.h"
#include "qRestAPI_p.h"

// --------------------------------------------------------------------------
void qRestAPIResult::setResult(QUuid queryUuid, const QList<QVariantMap>& result)
{
  this->QueryUuid = queryUuid;
  this->Result = result;
}

// --------------------------------------------------------------------------
void qRestAPIResult::setError(const QString& error)
{
  this->Error += error;
}

//// --------------------------------------------------------------------------
//void qRestAPIResult::setSslError(const QString& sslError)
//{
//  this->SslError += sslError;
//}

// --------------------------------------------------------------------------
// qRestAPIPrivate methods

// --------------------------------------------------------------------------
qRestAPIPrivate::qRestAPIPrivate(qRestAPI* object)
  : q_ptr(object)
{
  this->ResponseType = "json";
  this->NetworkManager = 0;
  this->TimeOut = 0;
  this->SuppressSslErrors = true;
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::init()
{
  Q_Q(qRestAPI);
  this->NetworkManager = new QNetworkAccessManager(q);
  QObject::connect(this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   this, SLOT(processReply(QNetworkReply*)));
#ifndef QT_NO_OPENSSL
//  if (QSslSocket::supportsSsl())
//    {
    QObject::connect(this->NetworkManager, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
          this, SLOT(onSslErrors(QNetworkReply*, QList<QSslError>)));
//    }
#endif
#if 0
  QObject::connect(q, SIGNAL(errorReceived(QString)),
                   this, SLOT(print(QString)));
  QObject::connect(q, SIGNAL(infoReceived(QString)),
                   this, SLOT(print(QString)));
#endif
}

// --------------------------------------------------------------------------
QUrl qRestAPIPrivate::createUrl(const QString& method, const qRestAPI::ParametersType& parameters)
{
//  return createUrlMidas(method, parameters);
  return createUrlXnat(method, parameters);
}

// --------------------------------------------------------------------------
QUrl qRestAPIPrivate::createUrlMidas(const QString& method, const qRestAPI::ParametersType& parameters)
{
  QUrl url(this->ServerUrl + "/api/" + this->ResponseType);
  if (!method.isEmpty())
    {
    url.addQueryItem("method", method);
    }
  foreach(const QString& parameter, parameters.keys())
    {
    url.addQueryItem(parameter, parameters[parameter]);
    }
  return url;
}

// --------------------------------------------------------------------------
QUrl qRestAPIPrivate::createUrlXnat(const QString& method, const qRestAPI::ParametersType& parameters)
{
  qDebug() << "qRestAPIPrivate::createUrlXnat(const QString& method, const qRestAPI::ParametersType& parameters)";
//  QUrl url(this->ServerUrl + "/REST" + method);
  QUrl url(this->ServerUrl + method);
  url.addQueryItem("format", this->ResponseType);
  qDebug() << url;
  foreach(const QString& parameter, parameters.keys())
    {
    url.addQueryItem(parameter, parameters[parameter]);
    }
  return url;
}

// --------------------------------------------------------------------------
QUuid qRestAPIPrivate::postQuery(const QUrl& url, const qRestAPI::RawHeadersType& rawHeaders)
{
  Q_Q(qRestAPI);
  QNetworkRequest queryRequest;
  queryRequest.setUrl(url);
  QUuid queryUuid = QUuid::createUuid();
  q->emit infoReceived("Post query: " + url.toString());

  for (QMapIterator<QByteArray, QByteArray> it(this->DefaultRawHeaders); it.hasNext();)
  {
    it.next();
    queryRequest.setRawHeader(it.key(), it.value());
  }

  for (QMapIterator<QByteArray, QByteArray> it(rawHeaders); it.hasNext();)
  {
    it.next();
    queryRequest.setRawHeader(it.key(), it.value());
  }

  QNetworkReply* queryReply = this->NetworkManager->get(queryRequest);
  queryReply->setProperty("uuid", queryUuid.toString());
  if (this->TimeOut > 0)
    {
    QTimer* timeOut = new QTimer(queryReply);
    timeOut->setSingleShot(true);
    QObject::connect(timeOut, SIGNAL(timeout()),
                     this, SLOT(queryTimeOut()));
    timeOut->start(this->TimeOut);
    QObject::connect(queryReply, SIGNAL(downloadProgress(qint64,qint64)),
                     this, SLOT(queryProgress()));
    }
  return queryUuid;
}

// --------------------------------------------------------------------------
QVariantMap qRestAPIPrivate::scriptValueToMap(const QScriptValue& value)
{
#if QT_VERSION >= 0x040700
  return value.toVariant().toMap();
#else
  QVariantMap result;
  for (QScriptValueIterator it(value); it.hasNext();)
    {
    it.next();
    result.insert(it.name(), it.value().toVariant());
    }
  return result;
#endif
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::appendScriptValueToVariantMapList(QList<QVariantMap>& result, const QScriptValue& data)
{
  QVariantMap map = scriptValueToMap(data);
  if (!map.isEmpty())
    {
    result << scriptValueToMap(data);
    }
}

// --------------------------------------------------------------------------
QList<QVariantMap> qRestAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
//  return parseResultMidas(scriptValue);
  return parseResultXnat(scriptValue);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qRestAPIPrivate::parseResultMidas(const QScriptValue& scriptValue)
{
  Q_Q(qRestAPI);
  // e.g. {"stat":"ok","code":"0","message":"","data":[{"p1":"v1","p2":"v2",...}]}
  QList<QVariantMap> result;
  QScriptValue stat = scriptValue.property("stat");
  if (stat.toString() != "ok")
    {
    QString error = QString("Error while parsing outputs:") +
      " status: " + scriptValue.property("stat").toString() + 
      " code: " + scriptValue.property("code").toInteger() + 
      " msg: " + scriptValue.property("message").toString();
    q->emit errorReceived(error);
    }
  QScriptValue data = scriptValue.property("data");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      q->emit errorReceived("No data");
      }
    else
      {
      q->emit errorReceived( QString("Bad data: ") + data.toString());
      }
    }
  if (data.isArray())
    {
    quint32 length = data.property("length").toUInt32();
    for(quint32 i = 0; i < length; ++i)
      {
      appendScriptValueToVariantMapList(result, data.property(i));
      }
    }
  else
    {
    appendScriptValueToVariantMapList(result, data);
    }
  
  return result;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qRestAPIPrivate::parseResultXnat(const QScriptValue& scriptValue)
{
  Q_Q(qRestAPI);
  // e.g. {"ResultSet":{"Result": [{"p1":"v1","p2":"v2",...}], "totalRecords":"13"}}
  QList<QVariantMap> result;
  QScriptValue resultSet = scriptValue.property("ResultSet");
//  if (stat.toString() != "ok")
//    {
//    QString error = QString("Error while parsing outputs:") +
//      " status: " + scriptValue.property("stat").toString() +
//      " code: " + scriptValue.property("code").toInteger() +
//      " msg: " + scriptValue.property("message").toString();
//    q->emit errorReceived(error);
//    }
  QScriptValue dataLength = resultSet.property("totalRecords");
  QScriptValue data = resultSet.property("Result");
  if (!data.isObject())
    {
    if (data.toString().isEmpty())
      {
      q->emit errorReceived("No data");
      }
    else
      {
      q->emit errorReceived( QString("Bad data: ") + data.toString());
      }
    }
  if (data.isArray())
    {
    quint32 length = data.property("length").toUInt32();
    for(quint32 i = 0; i < length; ++i)
      {
      appendScriptValueToVariantMapList(result, data.property(i));
      }
    }
  else
    {
    appendScriptValueToVariantMapList(result, data);
    }

  return result;
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::processReply(QNetworkReply* reply)
{
  Q_Q(qRestAPI);
  QUuid uuid(reply->property("uuid").toString());
  if (reply->error() != QNetworkReply::NoError)
    {
    q->emit errorReceived(uuid.toString() + ": "  + 
                  reply->error() + ": " +
                  reply->errorString());
    }
  QScriptValue scriptResult =
    this->ScriptEngine.evaluate("(" + QString(reply->readAll()) + ")");
  q->emit infoReceived(QString("Parse results for ") + uuid);
  QList<QVariantMap> result = this->parseResult(scriptResult);
//  QList<QVariantMap> result = this->parseXnatResult(scriptResult);
  q->emit infoReceived(QString("Results for ") + uuid.toString()
                       + ": " + q->qVariantMapListToString(result));
  q->emit resultReceived(uuid, result);
  reply->close();
  reply->deleteLater();
}

#ifndef QT_NO_OPENSSL
void qRestAPIPrivate::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)
{
  qDebug() << "qRestAPIPrivate::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors)";
  Q_Q(qRestAPI);
  QString errorString;
  foreach (const QSslError& error, errors)
  {
    if (!errorString.isEmpty())
    {
      errorString.append(", ");
    }
    errorString.append(error.errorString());
  }

  if (!this->SuppressSslErrors)
  {
    QString plural(errors.empty() ? "" : "s");
    QString error = QString("SSL error%1 has occurred: %2").arg(plural).arg(errorString);
    q->emit errorReceived(error);
  }
  else
  {
    reply->ignoreSslErrors();
  }
}
#endif

// --------------------------------------------------------------------------
void qRestAPIPrivate::print(const QString& msg)
{
  qDebug() << msg;
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::queryProgress()
{
  Q_Q(qRestAPI);
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(this->sender());
  Q_ASSERT(reply);
  if (!reply)
    {
    return;
    }
  // We received some progress so we postpone the timeout if any.
  QTimer* timer = reply->findChild<QTimer*>();
  if (timer)
    {
    timer->start();
    }
}

// --------------------------------------------------------------------------
void qRestAPIPrivate::queryTimeOut()
{
  Q_Q(qRestAPI);
  QTimer* timer = qobject_cast<QTimer*>(this->sender());
  Q_ASSERT(timer);
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(timer->parent());
  Q_ASSERT(reply);
  reply->abort();
  //reply->setError(QNetworkReply::TimeoutError,
  //   q->tr("Time out: No progress for %1 seconds.").arg(timer->interval()));
}

// --------------------------------------------------------------------------
// qRestAPI methods

// --------------------------------------------------------------------------
qRestAPI::qRestAPI(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qRestAPIPrivate(this))
{
  Q_D(qRestAPI);
  d->init();
  qRegisterMetaType<QUuid>("QUuid");
  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
}

qRestAPI::qRestAPI(qRestAPIPrivate* ptr, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(ptr)
{
  Q_D(qRestAPI);
  d->init();
  qRegisterMetaType<QUuid>("QUuid");
  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
}

// --------------------------------------------------------------------------
qRestAPI::~qRestAPI()
{
}

// --------------------------------------------------------------------------
QString qRestAPI::serverUrl()const
{
  Q_D(const qRestAPI);
  return d->ServerUrl;
}

// --------------------------------------------------------------------------
void qRestAPI::setServerUrl(const QString& newServerUrl)
{
  Q_D(qRestAPI);
  d->ServerUrl = newServerUrl;
}

// --------------------------------------------------------------------------
int qRestAPI::timeOut()const
{
  Q_D(const qRestAPI);
  return d->TimeOut;
}

// --------------------------------------------------------------------------
void qRestAPI::setTimeOut(int msecs)
{
  Q_D(qRestAPI);
  d->TimeOut = msecs;
}

// --------------------------------------------------------------------------
qRestAPI::RawHeadersType qRestAPI::defaultRawHeaders()const
{
  Q_D(const qRestAPI);
  return d->DefaultRawHeaders;
}

// --------------------------------------------------------------------------
void qRestAPI::setDefaultRawHeaders(const qRestAPI::RawHeadersType& defaultRawHeaders)
{
  Q_D(qRestAPI);
  d->DefaultRawHeaders = defaultRawHeaders;
}

// --------------------------------------------------------------------------
bool qRestAPI::suppressSslErrors()const
{
  Q_D(const qRestAPI);
  return d->SuppressSslErrors;
}

// --------------------------------------------------------------------------
void qRestAPI::setSuppressSslErrors(bool suppressSslErrors)
{
  Q_D(qRestAPI);
  d->SuppressSslErrors = suppressSslErrors;
}

// --------------------------------------------------------------------------
QUuid qRestAPI::query(const QString& method, const ParametersType& parameters, const qRestAPI::RawHeadersType& rawHeaders)
{
  Q_D(qRestAPI);
  QUrl url = d->createUrl(method, parameters);
  return d->postQuery(url, rawHeaders);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qRestAPI::synchronousQuery(
  bool &ok,
  const QString& serverUrl,const QString& method,
  const ParametersType& parameters,
  const RawHeadersType& rawHeaders,
  int maxWaitingTimeInMSecs)
{
  qRestAPI restAPI;
  restAPI.setServerUrl(serverUrl);
  restAPI.setSuppressSslErrors(true);
  restAPI.setTimeOut(maxWaitingTimeInMSecs);
  restAPI.query(method, parameters, rawHeaders);
  qRestAPIResult queryResult;
  QObject::connect(&restAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &queryResult, SLOT(setResult(QUuid,QList<QVariantMap>)));
  QObject::connect(&restAPI, SIGNAL(errorReceived(QString)),
                   &queryResult, SLOT(setError(QString)));
  QEventLoop eventLoop;
  QObject::connect(&restAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &eventLoop, SLOT(quit()));
  // Time out will fire an error which will quit the event loop.
  QObject::connect(&restAPI, SIGNAL(errorReceived(QString)),
                   &eventLoop, SLOT(quit()));
  eventLoop.exec();
  ok = queryResult.Error.isNull();
  if (!ok)
    {
    QVariantMap map;
    map["queryError"] = queryResult.Error;
    queryResult.Result.push_front(map);
    }
  if (queryResult.Result.count() == 0)
    {
    // \tbd
    Q_ASSERT(queryResult.Result.count());
    QVariantMap map;
    map["queryError"] = tr("Unknown error");
    queryResult.Result.push_front(map);
    }
  return queryResult.Result;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qRestAPI::synchronousQuery(
  bool &ok,
  const QString& method,
  const ParametersType& parameters,
  const RawHeadersType& rawHeaders,
  int maxWaitingTimeInMSecs)
{
  this->setSuppressSslErrors(true);
  this->setTimeOut(maxWaitingTimeInMSecs);
  qDebug() << " qRestAPI::synchronousQuery() method: " << method;
  this->query(method, parameters, rawHeaders);
  qRestAPIResult queryResult;
  QObject::connect(this, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &queryResult, SLOT(setResult(QUuid,QList<QVariantMap>)));
  QObject::connect(this, SIGNAL(errorReceived(QString)),
                   &queryResult, SLOT(setError(QString)));
  QEventLoop eventLoop;
  QObject::connect(this, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   &eventLoop, SLOT(quit()));
  // Time out will fire an error which will quit the event loop.
  QObject::connect(this, SIGNAL(errorReceived(QString)),
                   &eventLoop, SLOT(quit()));
  eventLoop.exec();
  ok = queryResult.Error.isNull();
  if (!ok)
    {
    QVariantMap map;
    map["queryError"] = queryResult.Error;
    queryResult.Result.push_front(map);
    }
  if (queryResult.Result.count() == 0)
    {
    // \tbd
    Q_ASSERT(queryResult.Result.count());
    QVariantMap map;
    map["queryError"] = tr("Unknown error");
    queryResult.Result.push_front(map);
    }
  return queryResult.Result;
}

// --------------------------------------------------------------------------
QString qRestAPI::qVariantMapListToString(const QList<QVariantMap>& list)
{
  QStringList values;
  foreach(const QVariantMap& map, list)
    {
    foreach(const QString& key, map.keys())
      {
      QString value;
      value += key;
      value += ": ";
      value += map[key].toString();
      values << value;
      }
    }
  return values.join("\n");
}
