/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CODEINFORMATIONREPOSITORY_H
#define CODEINFORMATIONREPOSITORY_H

#include "catalog.h"
#include <qmap.h>

#include <ktexteditor/codecompletioninterface.h>

class CodeInformationRepository
{
public:
    CodeInformationRepository();
    virtual ~CodeInformationRepository();

    const QMap<QString, Catalog*>& catalogs() const { return m_catalogs; }

    virtual void addCatalog( const QString& id, Catalog* catalog );
    virtual void removeCatalog( const QString& id );


    QValueList<KTextEditor::CompletionEntry> getEntriesInScope( const QStringList& scope );


    QValueList<Tag> query( const QValueList<Catalog::QueryArgument>& args );
    QValueList<Tag> getTagsInFile( const QString& fileName );
    QValueList<Tag> getBaseClassList( const QString& className );

private:
    QMap<QString, Catalog*> m_catalogs;

private:
   CodeInformationRepository( const CodeInformationRepository& source );
   void operator = ( const CodeInformationRepository& source );
};

#endif
