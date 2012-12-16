/*==============================================================================

  Program: qMidasAPI

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
#include <QEventLoop>

// qMidasAPI includes
#include "qMidasAPI.h"
#include "qMidasAPI_p.h"


// --------------------------------------------------------------------------
// qMidasAPIPrivate methods

// --------------------------------------------------------------------------
qMidasAPIPrivate::qMidasAPIPrivate(qMidasAPI* object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
QUrl qMidasAPIPrivate::createUrl(const QString& method, const qRestAPI::Parameters& parameters)
{
  QUrl url = Superclass::createUrl("/api/" + this->ResponseType, parameters);
  if (!method.isEmpty())
    {
    url.addQueryItem("method", method);
    }
  return url;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
  Q_Q(qMidasAPI);
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
// qMidasAPI methods

// --------------------------------------------------------------------------
qMidasAPI::qMidasAPI(QObject* _parent)
  : Superclass(new qMidasAPIPrivate(this), _parent)
{
}

// --------------------------------------------------------------------------
qMidasAPI::~qMidasAPI()
{
}

// --------------------------------------------------------------------------
QString qMidasAPI::midasUrl()const
{
  return Superclass::serverUrl();
}

// --------------------------------------------------------------------------
void qMidasAPI::setMidasUrl(const QString& newMidasUrl)
{
  Superclass::setServerUrl(newMidasUrl);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPI::synchronousQuery(
  bool &ok,
  const QString& serverUrl,
  const QString& method,
  const ParametersType& parameters,
  int maxWaitingTimeInMSecs)
{
  qMidasAPI restAPI;
  restAPI.setServerUrl(serverUrl);
  restAPI.setTimeOut(maxWaitingTimeInMSecs);
  restAPI.query(method, parameters);
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
