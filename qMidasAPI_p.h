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
#include "qMidasAPI.h"

// --------------------------------------------------------------------------
class qMidasAPIPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qMidasAPI);
  Q_OBJECT
protected:
  qMidasAPI* const q_ptr;
public:
  typedef qMidasAPIPrivate Self;
  qMidasAPIPrivate(qMidasAPI& object);

  virtual void init();

  QUrl createUrl(const QString& method, const qMidasAPI::ParametersType& parameters);
  QUuid postQuery(const QUrl& queryUrl, const qMidasAPI::RawHeadersType& rawHeaders = qMidasAPI::RawHeadersType());

  QList<QVariantMap> parseResult(const QScriptValue& scriptValue);
  QList<QVariantMap> parseXnatResult(const QScriptValue& scriptValue);
  QString qVariantMapToString(const QList<QVariantMap>& result)const;

public slots:
  void processReply(QNetworkReply* reply);
  void print(const QString& msg);
protected slots:
  /// Called when a query hasn't had any progress for a given TimeOut time.
  /// Note: sender() is used.
  void queryTimeOut();
  void queryProgress();
#ifndef QT_NO_OPENSSL
  void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
#endif

public:
  QString MidasUrl;
  QString ResponseType;

  QNetworkAccessManager* NetworkManager;
  QScriptEngine ScriptEngine;
  int TimeOut;
  qMidasAPI::RawHeadersType DefaultRawHeaders;
  bool SuppressSslErrors;
};

// --------------------------------------------------------------------------
class qMidasAPIResult : public QObject
{
  Q_OBJECT
public:
  QUuid QueryUuid;
  QList<QVariantMap> Result;
  QString Error;
//  QString SslError;
public slots:
  void setResult(QUuid queryUuid, const QList<QVariantMap>& result);
  void setError(const QString& error);
//  void setSslError(const QString& sslError);
};

#endif
