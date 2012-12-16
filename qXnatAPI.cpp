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

// qXnatAPI includes
#include "qXnatAPI.h"
#include "qXnatAPI_p.h"

// --------------------------------------------------------------------------
// qXnatAPIPrivate methods

// --------------------------------------------------------------------------
qXnatAPIPrivate::qXnatAPIPrivate(qXnatAPI* object)
  : Superclass(object)
{
}

// --------------------------------------------------------------------------
QUrl qXnatAPIPrivate
::createUrl(const QString& resource, const qRestAPI::Parameters& parameters)
{
  QUrl url = Superclass::createUrl(resource, parameters);
  url.addQueryItem("format", this->ResponseType);
  return url;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qXnatAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
  Q_Q(qXnatAPI);
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
// qXnatAPI methods

// --------------------------------------------------------------------------
qXnatAPI::qXnatAPI(QObject* _parent)
  : Superclass(new qXnatAPIPrivate(this), _parent)
{
}

// --------------------------------------------------------------------------
qXnatAPI::~qXnatAPI()
{
}
