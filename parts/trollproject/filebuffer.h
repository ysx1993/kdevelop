#ifndef _FILEBUFFER_H_
#define _FILEBUFFER_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include "caret.h"

class FileBuffer;
typedef QValueList<FileBuffer*> FileBufferList;

class FileBuffer
{
public:
  // Constructor/destructor
                  FileBuffer() {}
                  FileBuffer(const QString &fileName) {bufferFile(fileName);}
                  ~FileBuffer();

  // basic methods
  void            bufferFile(const QString &fileName);
  void            appendBufferText(QStringList buffer) {m_buffer+=buffer;}
  Caret           findInBuffer(const QString &subString,const Caret& startPos,bool nvlToMax=false);
  void            saveBuffer(const QString &filename);
  void            dumpBuffer();
  QString         pop(int row);
  QStringList     popBlock(const Caret &blockStart, const Caret &blockEnd);
  QStringList     copyBlock(const Caret &blockStart, const Caret &blockEnd);

  // Scopes
  void            setScopeName(const QString &scopeName) {m_scopeName=scopeName;}
  QString         getScopeName() {return m_scopeName;}
  bool            findNextScope(const Caret &pos, Caret& scopeStart, Caret& scopeEnd);
  Caret           findScopeEnd(Caret pos);

  // Recursive scope methods
  bool            handleScopes();
  int             findChildBuffer(const QString &scopeName);
  void            makeScope(const QString &scopeString);
  QStringList     getBufferTextInDepth();
  FileBuffer*     getSubBuffer(QString scopeString="");
  void            splitScopeString(QString scopeString,QString &scopeName, QString &scopeRest);
  QStringList     getAllScopeStrings(int depth=0);
  QStringList     getAllScopeNames(int depth=0);

  // Variable value handling
  void            removeValues(const QString &variable);
  QString         getValues(const QString &variable);
  void            setValues(const QString &variable,QString values,int valuesPerRow=3)
                  {setValues(variable,QStringList::split(' ',values),valuesPerRow);}
  void            setValues(const QString &variable,QStringList values,int valuesPerRow=3);

private:
  QString         m_scopeName;
  QStringList     m_buffer;
  FileBufferList  m_subBuffers;
};

#endif
