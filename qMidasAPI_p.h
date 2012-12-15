/*==============================================================================

  Program: 3D Slicer

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

#ifndef __qMidasAPI_p_h
#define __qMidasAPI_p_h

// Qt includes
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#ifndef QT_NO_OPENSSL
  #include <QSslError>
#endif
#include <QScriptEngine>

// qMidasAPI includes
#include "qRestAPI_p.h"
#include "qMidasAPI.h"

// --------------------------------------------------------------------------
class qMidasAPIPrivate : public qRestAPIPrivate
{
  Q_DECLARE_PUBLIC(qMidasAPI);
  Q_OBJECT

  typedef qRestAPIPrivate Superclass;

protected:
  qMidasAPI* const q_ptr;
public:
  typedef qMidasAPIPrivate Self;
  qMidasAPIPrivate(qMidasAPI* object);

  QUrl createUrl(const QString& method, const qMidasAPI::ParametersType& parameters);
  QList<QVariantMap> parseResult(const QScriptValue& scriptValue);
};

#endif
