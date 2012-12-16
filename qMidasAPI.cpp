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
#include <QDebug>
#include <QEventLoop>
#include <QScriptValueIterator>
#include <QSslSocket>
#include <QStringList>
#include <QTimer>
#include <QUuid>
#include <QDebug>

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
QUrl qMidasAPIPrivate::createUrl(const QString& method, const qRestAPI::ParametersType& parameters)
{
  qDebug() << "qMidasAPIPrivate::createUrl(const QString& method, const qMidasAPI::ParametersType& parameters)";
  return createUrlMidas(method, parameters);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
  qDebug() << "qMidasAPIPrivate::parseResult(const QScriptValue& scriptValue)";
  return parseResultMidas(scriptValue);
}

// --------------------------------------------------------------------------
QList<QVariantMap> qMidasAPIPrivate::parseResultMidas(const QScriptValue& scriptValue)
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
