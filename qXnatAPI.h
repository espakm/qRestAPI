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

#ifndef __qXnatAPI_h
#define __qXnatAPI_h

#include "qRestAPI.h"

class qXnatAPIPrivate;

/// qXnatAPI is a simple interface class to communicate with a Xnat3 public
/// API.
/// Queries are posted to the server and answers reported back.
/// qXnatAPI works in synchronous or unsynchronous way.
/// Usage:
/// <code>
/// qXnatAPI midas;
/// midas.setXnatUrl("http://slicer.kitware.com/midas3");
/// connect(&midas, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
///         myApp, SLOT(processResult(QUuid,QList<QVariantMap>)));
/// midas.query("midas.version");
/// ...
/// </code>
class qXnatAPI : public qRestAPI
{
  Q_OBJECT

public:
  typedef qRestAPI Superclass;
  explicit qXnatAPI(QObject*parent = 0);
  virtual ~qXnatAPI();

  typedef QMap<QString, QString> ParametersType;
  typedef QMap<QByteArray, QByteArray> RawHeadersType;

private:
  Q_DECLARE_PRIVATE(qXnatAPI);
  Q_DISABLE_COPY(qXnatAPI);
};

#endif
