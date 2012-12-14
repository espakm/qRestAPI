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

#ifndef __qXnatAPI_p_h
#define __qXnatAPI_p_h

// Qt includes
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#ifndef QT_NO_OPENSSL
  #include <QSslError>
#endif
#include <QScriptEngine>

// qXnatAPI includes
#include "qRestAPI_p.h"
#include "qXnatAPI.h"

// --------------------------------------------------------------------------
class qXnatAPIPrivate : public qRestAPIPrivate
{
  Q_DECLARE_PUBLIC(qXnatAPI);
  Q_OBJECT

  typedef qRestAPIPrivate Superclass;

protected:
  qXnatAPI* const q_ptr;
public:
  typedef qXnatAPIPrivate Self;
  qXnatAPIPrivate(qXnatAPI& object);

  QUrl createUrl(const QString& method, const qXnatAPI::ParametersType& parameters);
  QList<QVariantMap> parseResult(const QScriptValue& scriptValue);
};

#endif
