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

/// qXnatAPI is a simple interface class to communicate with an XNAT
/// server through its REST API.
class qXnatAPI : public qRestAPI
{
  Q_OBJECT

  typedef qRestAPI Superclass;

public:
  explicit qXnatAPI(QObject*parent = 0);
  virtual ~qXnatAPI();

private:
  Q_DECLARE_PRIVATE(qXnatAPI);
  Q_DISABLE_COPY(qXnatAPI);
};

#endif
