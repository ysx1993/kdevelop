/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "oktetaplugin.h"

// plugin
#include "oktetadocumentfactory.h"
#include "oktetatoolviewfactory.h"
#include "oktetadocument.h"
// Okteta Kasten tools
#include <Kasten/Okteta/StringsExtractToolViewFactory>
#include <Kasten/Okteta/StringsExtractToolFactory>
#include <Kasten/Okteta/InfoToolViewFactory>
#include <Kasten/Okteta/InfoToolFactory>
#include <Kasten/Okteta/FilterToolViewFactory>
#include <Kasten/Okteta/FilterToolFactory>
#include <Kasten/Okteta/ChecksumToolViewFactory>
#include <Kasten/Okteta/ChecksumToolFactory>
// #include <documentinfotoolview.h>
// #include <documentinfotool.h>
#include <Kasten/Okteta/PODDecoderToolViewFactory>
#include <Kasten/Okteta/PODDecoderToolFactory>
#include <Kasten/Okteta/ByteTableToolViewFactory>
#include <Kasten/Okteta/ByteTableToolFactory>
#include <Kasten/Okteta/BookmarksToolViewFactory>
#include <Kasten/Okteta/BookmarksToolFactory>
#ifndef BIG_ENDIAN
#include <Kasten/Okteta/StructuresToolViewFactory>
#include <Kasten/Okteta/StructuresToolFactory>
#endif
// Okteta Kasten
#include <Kasten/Okteta/ByteArrayViewProfileManager>
// KDev
#include <project/projectmodel.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/context.h>
// KF
#include <KPluginFactory>
#include <KLocalizedString>

#include <QMimeType>
#include <QAction>

K_PLUGIN_FACTORY_WITH_JSON(OktetaPluginFactory, "kdevokteta.json", registerPlugin<KDevelop::OktetaPlugin>(); )

namespace KDevelop
{

static inline
void addTool( IUiController* uiController,
              Kasten::AbstractToolViewFactory* toolViewFactory,
              Kasten::AbstractToolFactory* toolFactory )
{
    auto* factory =
        new OktetaToolViewFactory( toolViewFactory, toolFactory );

    uiController->addToolView( toolViewFactory->title(), factory );
}


OktetaPlugin::OktetaPlugin( QObject* parent, const QVariantList& args )
  : IPlugin( QStringLiteral("kdevokteta"), parent )
  , mDocumentFactory( new OktetaDocumentFactory(this) )
  , mViewProfileManager( new Kasten::ByteArrayViewProfileManager() )
{
    Q_UNUSED(args)

    IUiController* uiController = core()->uiController();
    addTool( uiController, new Kasten::ChecksumToolViewFactory(), new Kasten::ChecksumToolFactory() );
    addTool( uiController, new Kasten::FilterToolViewFactory(), new Kasten::FilterToolFactory() );
    addTool( uiController, new Kasten::StringsExtractToolViewFactory, new Kasten::StringsExtractToolFactory() );
    addTool( uiController, new Kasten::ByteTableToolViewFactory(), new Kasten::ByteTableToolFactory() );
    addTool( uiController, new Kasten::InfoToolViewFactory(), new Kasten::InfoToolFactory() );
    addTool( uiController, new Kasten::PodDecoderToolViewFactory(), new Kasten::PodDecoderToolFactory() );
// disable Okteta Structures tool on big-endian as it's disable in kdesdk
#ifndef BIG_ENDIAN
    addTool( uiController, new Kasten::StructuresToolViewFactory(), new Kasten::StructuresToolFactory() );
#endif
    addTool( uiController, new Kasten::BookmarksToolViewFactory, new Kasten::BookmarksToolFactory() );

    KDevelop::IDocumentController* documentController = core()->documentController();
    documentController->registerDocumentForMimetype(QStringLiteral("application/octet-stream"), mDocumentFactory);
}

ContextMenuExtension OktetaPlugin::contextMenuExtension(Context* context, QWidget* parent)
{
    auto* openWithContext = dynamic_cast<OpenWithContext*>( context );

    if( openWithContext && !openWithContext->mimeType().inherits(QStringLiteral("inode/directory")))
    {
        QAction* openAction = new QAction(i18n("Hex Editor"), parent);
        openAction->setIcon( QIcon::fromTheme(QStringLiteral("document-open")) );
        openAction->setData( QVariant::fromValue(openWithContext->urls()) );
        connect( openAction, &QAction::triggered, this, &OktetaPlugin::onOpenTriggered );

        KDevelop::ContextMenuExtension contextMenuExtension;
        contextMenuExtension.addAction( KDevelop::ContextMenuExtension::OpenEmbeddedGroup, openAction );
        return contextMenuExtension;
    }

    return KDevelop::IPlugin::contextMenuExtension(context, parent);
}

void OktetaPlugin::onOpenTriggered()
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);

    KDevelop::ICore* core = KDevelop::ICore::self();
    IDocumentController* documentController = core->documentController();

    foreach( const QUrl &url, action->data().value<QList<QUrl>>() )
    {
        IDocument* existingDocument = documentController->documentForUrl(url);
        if( existingDocument )
            if( ! existingDocument->close() )
                continue;

        IDocument* createdDocument = mDocumentFactory->create( url, core );
        if( createdDocument )
            documentController->openDocument( createdDocument );
    }
}

OktetaPlugin::~OktetaPlugin()
{
    delete mDocumentFactory;
}

}

#include "oktetaplugin.moc"
