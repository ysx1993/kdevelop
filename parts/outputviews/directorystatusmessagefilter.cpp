/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "directorystatusmessagefilter.h"
#include "directorystatusmessagefilter.moc"
#include "makeitem.h"

#include <qregexp.h>

DirectoryStatusMessageFilter::DirectoryStatusMessageFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

void DirectoryStatusMessageFilter::processLine( const QString& line )
{
	QString dir;
	if ( matchEnterDir( line, dir ) )
	{
		emit item( new EnteringDirectoryItem( dir, line ) );
	}
	else if ( matchLeaveDir( line, dir ) )
	{
		emit item( new ExitingDirectoryItem( dir, line ) );
	}
	else
	{
		OutputFilter::processLine( line );
	}
}

// simple helper function - checks whether we entered a new directory
// (locale and Utf8 aware)
bool DirectoryStatusMessageFilter::matchEnterDir( const QString& line, QString& dir )
{
    // make outputs localised strings in Utf8 for entering/leaving directories...
    static const unsigned short fr_enter[] =
        {'E','n','t','r','e',' ','d','a','n','s',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e'
        };
    static const unsigned short pl_enter[] =
        {'W','c','h','o','d','z',0x119,' ','k','a','t','a','l','o','g'
        };
    static const unsigned short ja_enter[] =
        {
            0x5165,0x308a,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea
        };
    static const unsigned short ko_enter[] =
        {
            0xb4e4,0xc5b4,0xac10
        };
    static const unsigned short ko_behind[] =
        {
            0x20,0xb514,0xb809,0xd1a0,0xb9ac
        };
    static const unsigned short pt_br_enter[] =
        {
            0x45,0x6e,0x74,0x72,0x61,0x6e,0x64,0x6f,0x20,0x6e,0x6f,0x20,0x64,0x69,0x72,0x65,0x74,0xf3,0x72,0x69,0x6f
        };
    static const unsigned short ru_enter[] =
        {
            0x412,0x445,0x43e,0x434,0x20,0x432,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433
        };

    static const QString fr_e( (const QChar*)fr_enter, sizeof(fr_enter) / 2 );
    static const QString pl_e( (const QChar*)pl_enter, sizeof(pl_enter) / 2 );
    static const QString ja_e( (const QChar*)ja_enter, sizeof(ja_enter) / 2 );
    static const QString ko_e( (const QChar*)ko_enter, sizeof(ko_enter) / 2 );
    static const QString ko_b( (const QChar*)ko_behind, sizeof(ko_behind) / 2 );
    static const QString pt_br_e( (const QChar*)pt_br_enter, sizeof(pt_br_enter) / 2 );
    static const QString ru_e( (const QChar*)ru_enter, sizeof(ru_enter) / 2 );

    // we need a QRegExp because KRegExp is not Utf8 aware.
    // 0x00AB is LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    // 0X00BB is RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    static QRegExp dirChange(QString::fromLatin1(".*: (.+) (`|") + QChar(0x00BB) + QString::fromLatin1(")(.*)('|") + QChar(0x00AB) + QString::fromLatin1(")(.*)"));

    if (dirChange.search(line) > -1 )
    {
        QString msg = dirChange.cap(1);
        QString msgBehind = dirChange.cap(5);

        if ( msg == "Entering directory" ||   // English - default
                msg == "Wechsel in das Verzeichnis Verzeichnis" ||    // German - yes, this is badly translated
		msg == "Wechsel in das Verzeichnis" || // German without typo
                msg == "Cambiando a directorio" || // Spanish
                msg == fr_e || // French
                msg == ja_e || // Japanese
                ( msg == ko_e && msgBehind == ko_b ) || // Korean
                msg == "Binnengaan van directory" || // Dutch
                msg == pl_e || // Polish
                msg == pt_br_e || // Brazilian Portuguese
                msg == ru_e ) // Russian
        {
            dir = dirChange.cap(3);
            return true;
        }
    }

    return false;
}

// simple helper function - checks whether we left a directory
// (locale and Utf8 aware).
bool DirectoryStatusMessageFilter::matchLeaveDir( const QString& line, QString& dir )
{
    static const unsigned short fr_leave[] =
        { 'Q','u','i','t','t','e',' ','l','e',' ','r',0xe9,'p','e','r','t','o','i','r','e'
        };
    static const unsigned short ja_leave[] =
        {
            0x51fa,0x307e,0x3059,0x20,0x30c7,0x30a3,0x30ec,0x30af,0x30c8,0x30ea
        };
    static const unsigned short pt_br_leave[] =
        {'S','a','i','n','d','o',' ','d','o',' ','d','i','r','e','t',0xf3,'r','i','o'
        };
    static const unsigned short ru_leave[] =
        {
            0x412,0x44b,0x445,0x43e,0x434,0x20,0x438,0x437,0x20,0x43a,0x430,0x442,0x430,0x43b,0x43e,0x433
        };
    static const unsigned short ko_leave[] =
        {
            0xb098,0xac10
        };
    static const unsigned short ko_behind[] =
        {
            0x20,0xb514,0xb809,0xd1a0,0xb9ac
        };

    static const QString fr_l( (const QChar*)fr_leave, sizeof(fr_leave) / 2 );
    static const QString ja_l( (const QChar*)ja_leave, sizeof(ja_leave) / 2 );
    static const QString ko_l( (const QChar*)ko_leave, sizeof(ko_leave) / 2 );
    static const QString pt_br_l( (const QChar*)pt_br_leave, sizeof(pt_br_leave) / 2 );
    static const QString ru_l( (const QChar*)ru_leave, sizeof(ru_leave) / 2 );

    // we need a QRegExp because KRegExp is not Utf8 aware.
    // 0x00AB is LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    // 0X00BB is RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    static QRegExp dirChange(QString::fromLatin1(".*: (.+) (`|") + QChar(0x00BB) + QString::fromLatin1(")(.*)('|") + QChar(0x00AB) + QString::fromLatin1(")(.*)"));

    if (dirChange.search(line) > -1 )
    {
        QString msg = dirChange.cap(1);
//        QString msgBehind = dirChange.cap(5);

        if ( msg == "Leaving directory" || // en
                msg == "Verlassen des Verzeichnisses Verzeichnis" || // de
		msg == "Verlassen des Verzeichnisses" ||
                msg == "Saliendo directorio" || // es
                msg == fr_l || // fr
                msg == ja_l || //ja
                ( msg == (const char*)ko_leave && msg == (const char*)ko_behind ) || //ko
                msg == "Verdwijnen uit directory" || //nl
                msg == "Opuszczam katalog" || //po
                msg == pt_br_l || //pt_BR
                msg == ru_l ) //ru
        {
            dir = dirChange.cap(3);
            return true;
        }
    }

    return false;
}
