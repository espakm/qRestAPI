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
::createUrl(const QString& method, const qRestAPI::ParametersType& parameters)
{
  qDebug() << "qXnatAPIPrivate::createUrl(const QString& method, const qRestAPI::ParametersType& parameters)";
  QUrl url(this->ServerUrl + "/REST" + method);
//  QUrl url(this->ServerUrl + method);
  url.addQueryItem("format", this->ResponseType);
  qDebug() << url;
  foreach(const QString& parameter, parameters.keys())
    {
    url.addQueryItem(parameter, parameters[parameter]);
    }
  return url;
}

// --------------------------------------------------------------------------
QList<QVariantMap> qXnatAPIPrivate::parseResult(const QScriptValue& scriptValue)
{
  return parseResultXnat(scriptValue);
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
