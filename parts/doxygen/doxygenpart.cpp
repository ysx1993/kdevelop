/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004 by Jonas Jacobi                                    *
 *    jonas.jacobi@web.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doxygenpart.h"
#include "doxygenconfigwidget.h"
#include "configwidgetproxy.h"
#include "config.h"
#include "kdevappfrontend.h"

#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevmakefrontend.h>
#include <kdevcore.h>
#include <codemodel.h>
#include <codemodel_utils.h>
#include <domutil.h>

#include <kdebug.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kmainwindow.h>
#include <kparts/part.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/editinterface.h>
#include <partcontroller.h>
#include <kdialogbase.h>
#include <kdevplugininfo.h>

#include <qvbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qfileinfo.h>

#define PROJECTOPTIONS 1

typedef KDevGenericFactory<DoxygenPart> DoxygenFactory;
static const KDevPluginInfo data("kdevdoxygen");
K_EXPORT_COMPONENT_FACTORY( libkdevdoxygen, DoxygenFactory( data ) )

DoxygenPart::DoxygenPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(&data, parent, name ? name : "DoxygenPart"), m_activeEditor(0), m_cursor(0)
{
    setInstance(DoxygenFactory::instance());
    setXMLFile("kdevdoxygen.rc");

    KAction *action;
    action = new KAction( i18n("Build API Documentation"), 0,
                          this, SLOT(slotDoxygen()),
                          actionCollection(), "build_doxygen" );
    action->setToolTip(i18n("Build API documentation"));
    action->setWhatsThis(i18n("<b>Build API documentation</b><p>Runs doxygen on a project Doxyfile to generate API documentation. "
        "If the search engine is enabled in Doxyfile, this also runs doxytag to create it."));

    action = new KAction( i18n("Clean API Documentation"), 0,
                          this, SLOT(slotDoxClean()),
                          actionCollection(), "clean_doxygen" );
    action->setToolTip(i18n("Clean API documentation"));
    action->setWhatsThis(i18n("<b>Clean API documentation</b><p>Removes all generated by doxygen files."));

//    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );

	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createProjectConfigPage( i18n("Doxygen"), PROJECTOPTIONS, info()->icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );

    m_actionDocumentFunction = new KAction(i18n("Document Current Function"), 0, CTRL+SHIFT+Key_S, this, SLOT(slotDocumentFunction()), actionCollection(), "edit_document_function");
    m_actionDocumentFunction->setToolTip( i18n("Create a documentation template above a function"));
    m_actionDocumentFunction->setWhatsThis(i18n("<b>Document Current Function</b><p>Creates a documentation template according to a function's signature above a function definition/declaration."));

    m_tmpDir.setAutoDelete(true);
    connect( partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part* )));
    m_actionPreview = new KAction(i18n("Preview Doxygen Output"), 0, CTRL+ALT+Key_P, this, SLOT(slotRunPreview()), actionCollection(), "show_preview_doxygen_output");
    m_actionPreview->setToolTip( i18n("Show a preview of the Doxygen output of this file") );
    m_actionPreview->setWhatsThis( i18n("<b>Preview Doxygen output</b><p>Runs Doxygen over the current file and shows the created index.html.") );

    //read Doxygen configuration, if none exists yet, create it with some defaults
    adjustDoxyfile();
    QString fileName = project()->projectDirectory() + "/Doxyfile";

    QFile file(fileName);
    if (file.open(IO_ReadOnly)) {
        QTextStream is(&file);

        Config::instance()->parse(QFile::encodeName(fileName));
        Config::instance()->convertStrToVal();

        file.close();
    }
}


DoxygenPart::~DoxygenPart()
{
	delete _configProxy;
}

void DoxygenPart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	if ( pagenumber == PROJECTOPTIONS )
	{
		adjustDoxyfile();

		DoxygenConfigWidget *w = new DoxygenConfigWidget(project()->projectDirectory() + "/Doxyfile", page );
		connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
	}
}

/** If a Doxygen configuration file doesn't exist, create one.
  * And copy some of the project settings to it.
  */
void DoxygenPart::adjustDoxyfile()
{
  QString fileName = project()->projectDirectory() + "/Doxyfile";
  if (QFile::exists(fileName))
    return;

  // Initialize configuration
  Config::instance()->init();

  // Do some checks and improve the configuration a bit
  Config::instance()->check();

  // set "General/PROJECT_NAME"
  ConfigString *name = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NAME"));
  if (name)
  {
    name->setDefaultValue(project()->projectName().latin1());
    name->init();
  }

  // set "General/PROJECT_NUMBER"
  ConfigString *version = dynamic_cast<ConfigString*>(Config::instance()->get("PROJECT_NUMBER"));
  if (version)
  {
    version->setDefaultValue(DomUtil::readEntry(*projectDom(), "/general/version").latin1());
    version->init();
  }

  // insert input files into "Input/INPUT"
  ConfigList *input_files = dynamic_cast<ConfigList*>(Config::instance()->get("INPUT"));
  if (input_files)
  {
    input_files->init();
    input_files->addValue(QFile::encodeName(project()->projectDirectory()));
  }

  // insert file patterns into "Input/FILE_PATTERNS"
  ConfigList *patterns = dynamic_cast<ConfigList*>(Config::instance()->get("FILE_PATTERNS"));
  if (patterns)
  {
    // Remove Doxygen's default patterns
//    patterns->init();

    // Add this ones:
    patterns->addValue("*.C");
    patterns->addValue("*.H");
    patterns->addValue("*.tlh");
    patterns->addValue("*.diff");
    patterns->addValue("*.patch");
    patterns->addValue("*.moc");
    patterns->addValue("*.xpm");
    patterns->addValue("*.dox");
  }

  // set "Input/RECURSIVE" to recurse into subdirectories
  ConfigBool *recursive = dynamic_cast<ConfigBool*>(Config::instance()->get("RECURSIVE"));
  if (recursive)
  {
    recursive->setValueString("yes");
  }

  // set "XML/GENERATE_XML" to generate XML information to be used with code hinting
  ConfigBool *gen_xml = dynamic_cast<ConfigBool*>(Config::instance()->get("GENERATE_XML"));
  if (gen_xml)
  {
    gen_xml->setValueString("yes");
  }

  // set "Enternal/GENERATE_TAGFILE" to generate tag file for documentation browser
  ConfigString *gen_tag = dynamic_cast<ConfigString*>(Config::instance()->get("GENERATE_TAGFILE"));
  if (gen_tag)
  {
    gen_tag->setDefaultValue(QString(project()->projectName()+".tag").latin1());
    gen_tag->init();
  }

  // write doxy file
  QFile f2(fileName);
  if (!f2.open(IO_WriteOnly))
    KMessageBox::information(mainWindow()->main(), i18n("Cannot write Doxyfile."));
  else
  {
    QTextStream ts_file(&f2);

    Config::instance()->writeTemplate(ts_file, true, true);

    f2.close();
  }
}


void DoxygenPart::slotDoxygen()
{
    bool searchDatabase = false;
    QString outputDirectory;
    QString htmlDirectory;

    adjustDoxyfile();

    QString fileName = project()->projectDirectory() + "/Doxyfile";

    Config::instance()->init();

    QFile f(fileName);
    if (f.open(IO_ReadOnly))
    {
      QTextStream is(&f);

      Config::instance()->parse(QFile::encodeName(fileName));
      Config::instance()->convertStrToVal();

      f.close();
    }

    // search engine
    ConfigBool *search = dynamic_cast<ConfigBool*>(Config::instance()->get("SEARCHENGINE"));
    if (search)
    {
      searchDatabase = Config_getBool("SEARCHENGINE");

      if (searchDatabase)
      {
        // get input files
        outputDirectory = Config_getString("OUTPUT_DIRECTORY");
        if ( outputDirectory.isEmpty() == false )
          outputDirectory += "/";
        htmlDirectory   = Config_getString("HTML_OUTPUT");
        if ( htmlDirectory.isEmpty() == true )
          htmlDirectory = "html";
        htmlDirectory.prepend(outputDirectory);
      }
    }

    QString dir = project()->projectDirectory();
    QString cmdline = "cd ";
    cmdline += KShellProcess::quote( dir );
    cmdline += " && doxygen Doxyfile";
    if (searchDatabase)
    {
      // create search database in the same directory where the html docs are
      if ( htmlDirectory.length() > 0 )
        cmdline += " && cd " + KShellProcess::quote( htmlDirectory );
      cmdline += " && doxytag -s search.idx ";
    }

    kdDebug(9026) << "Doxygen command line: " << cmdline << endl;

    if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
        makeFrontend->queueCommand(dir, cmdline);
}


void DoxygenPart::slotDoxClean()
{
    bool could_be_dirty = false;

    QString outputDirectory = Config_getString("OUTPUT_DIRECTORY");
    if ( outputDirectory.isEmpty() )
        outputDirectory = project()->projectDirectory();
    if ( outputDirectory.right(1) != "/" )
        outputDirectory += "/";
    QString cmdline = "cd " + KShellProcess::quote( outputDirectory );

    if ( Config_getBool("GENERATE_HTML") ) {
        QString htmlDirectory   = Config_getString("HTML_OUTPUT");
        if ( htmlDirectory.isEmpty() )
            htmlDirectory = "html";
        if ( htmlDirectory.right(1) != "/" )
            htmlDirectory += "/";
        cmdline += " && rm -f " + KShellProcess::quote( htmlDirectory ) + "*";
        could_be_dirty= true;
    }

    if ( Config_getBool("GENERATE_LATEX") ) {
        QString latexDirectory   = Config_getString("LATEX_OUTPUT");
        if ( latexDirectory.isEmpty() )
            latexDirectory = "latex";
        if ( latexDirectory.right(1) != "/" )
            latexDirectory += "/";
        cmdline += " && rm -f " + KShellProcess::quote( latexDirectory ) + "*";
        could_be_dirty= true;
    }

    if ( Config_getBool("GENERATE_RTF") ) {
        QString rtfDirectory   = Config_getString("RTF_OUTPUT");
        if ( rtfDirectory.isEmpty() )
            rtfDirectory = "rtf";
        if ( rtfDirectory.right(1) != "/" )
            rtfDirectory += "/";
        cmdline += " && rm -f " + KShellProcess::quote( rtfDirectory ) + "*";
        could_be_dirty= true;
    }

    if ( Config_getBool("GENERATE_MAN") ) {
        QString manDirectory   = Config_getString("MAN_OUTPUT");
        if ( manDirectory.isEmpty() )
            manDirectory = "man";
        if ( manDirectory.right(1) != "/" )
            manDirectory += "/";
        cmdline += " && rm -f " + KShellProcess::quote( manDirectory ) + "*";
        could_be_dirty= true;
    }

    if ( Config_getBool("GENERATE_XML") ) {
        QString xmlDirectory   = Config_getString("XML_OUTPUT");
        if ( xmlDirectory.isEmpty() )
            xmlDirectory = "xml";
        if ( xmlDirectory.right(1) != "/" )
            xmlDirectory += "/";
        cmdline += " && rm -f " + KShellProcess::quote( xmlDirectory ) + "*";
        could_be_dirty= true;
    }

    if (could_be_dirty) {
        kdDebug(9026) << "Cleaning Doxygen generated API documentation using: " << cmdline << endl;
        if (KDevMakeFrontend *makeFrontend = extension<KDevMakeFrontend>("KDevelop/MakeFrontend"))
            makeFrontend->queueCommand(KShellProcess::quote(project()->projectDirectory()), cmdline);
    }
    else
       kdDebug(9026) << "No Doxygen generated API documentation exists. There's nothing to clean!" << endl;

}

void DoxygenPart::slotPreviewProcessExited( )
{
    KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend");
    if ( appFrontend != 0 )
        disconnect(appFrontend, 0, this, 0);
    partController()->showDocument(KURL(m_tmpDir.name()+"html/index.html"));
}

void DoxygenPart::slotRunPreview( )
{
    if (m_file.isNull())
        return;

    KDevAppFrontend *appFrontend = extension<KDevAppFrontend>("KDevelop/AppFrontend");
    if ( appFrontend == 0 )
        return;

    if ( appFrontend->isRunning() ) {
        KMessageBox::information( mainWindow()->main(),
            i18n("Another process is still running. Please wait until it's finished."));
        return;
    }

    m_tmpDir.unlink();
    m_tmpDir = KTempDir();
    m_tmpDir.setAutoDelete(true);

    Config* config = Config::instance();

    ConfigString* poDir = dynamic_cast<ConfigString*>(config->get("OUTPUT_DIRECTORY"));
    ConfigList* pInput = dynamic_cast<ConfigList*>(config->get("INPUT"));
    ConfigString* pHeader = dynamic_cast<ConfigString*>(config->get("HTML_HEADER"));
    ConfigString* pFooter = dynamic_cast<ConfigString*>(config->get("HTML_FOOTER"));
    ConfigString* pStyle = dynamic_cast<ConfigString*>(config->get("HTML_STYLESHEET"));

    //store config values to restore them later | override config values to get only the current file processed
    QCString dirVal;
    if (poDir != 0) {
        dirVal = *poDir->valueRef();
        *poDir->valueRef() = m_tmpDir.name().ascii();
    }

   QStrList inputVal;
    if (pInput != 0) {
        inputVal = *pInput->valueRef();
         QStrList xl;
         xl.append(m_file.ascii());
        *pInput->valueRef() = xl;
    } else {
        config->addList("INPUT", "# The INPUT tag can be used to specify the files and/or directories that contain\n"
                                                     "# documented source files. You may enter file names like \"myfile.cpp\" or\n"
                                                     "# directories like \"/usr/src/myproject\". Separate the files or directories\n"
                                                     "# with spaces.");
        pInput = dynamic_cast<ConfigList*>(config->get("INPUT")); //pinput now has to be != 0
        QStrList xl;
         xl.append(m_file.ascii());
        *pInput->valueRef() = xl;
    }

    QCString header;
    QCString footer;
    QCString stylesheet;
    //if header/footer/stylesheets are set, make sure they get found in the doxygen run
    QString projectDir = project()->projectDirectory();
    if (pHeader != 0 && !pHeader->valueRef()->isEmpty()){
        header = *pHeader->valueRef();
        QFileInfo info (header);
        if (info.isRelative())
            *pHeader->valueRef() = QString(projectDir + "/" + QString(header)).ascii();
        else
            header = 0;
    }

    if (pFooter != 0 && !pFooter->valueRef()->isEmpty()){
        footer = *pFooter->valueRef();
        QFileInfo info (footer);
        if (info.isRelative())
            *pFooter->valueRef() = QString(projectDir + "/" + QString(footer)).ascii();
        else
            footer = 0;
    }

    if (pStyle != 0 && !pStyle->valueRef()->isEmpty()){
        stylesheet = *pStyle->valueRef();
        QFileInfo info (stylesheet);
        if (info.isRelative())
            *pStyle->valueRef() = QString(projectDir +"/" + QString(stylesheet)).ascii();
        else
            stylesheet = 0;
    }

    QFile file(m_tmpDir.name() +"PreviewDoxyfile"); //file gets deleted automatically 'cause of tempdir
    if (!file.open(IO_WriteOnly)){
        //restore config values
        if (pInput != 0)
            *pInput->valueRef() = inputVal;

        if (poDir != 0)
            *poDir->valueRef() = dirVal;

        KMessageBox::error(mainWindow()->main(), i18n("Cannot create temporary file '%1'").arg(file.name()));
        return;
    }

    QTextStream ts_file(&file);

    config->writeTemplate(ts_file, false, false);
    file.close();

    if (inputVal.count() == 0) //pInput is always != 0
        *pInput->valueRef() = QStrList();
    else
        *pInput->valueRef() = inputVal;

    if (poDir != 0)
        *poDir->valueRef() = dirVal;

    if (pHeader != 0 && !header.isNull())
        *pHeader->valueRef() = header;

    if (pFooter != 0 && !footer.isNull())
        *pFooter->valueRef() = footer;

    if (pStyle != 0 && !stylesheet.isNull())
        *pStyle->valueRef() = stylesheet;

    connect(appFrontend, SIGNAL(processExited()), this, SLOT(slotPreviewProcessExited()));
    appFrontend->startAppCommand("", "doxygen \"" + file.name() + "\"", false);
}

void DoxygenPart::slotActivePartChanged( KParts::Part * part )
{
	// -> idea from cppsupportpart.cpp
	KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>(part);
	if (doc != 0)
		m_file = doc->url().path();
	else
		m_file = QString::null;
	// <-
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>(part);
    m_cursor = part ? dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget()) : 0;
}

void DoxygenPart::slotDocumentFunction(){
    if (m_activeEditor != 0 && m_cursor != 0){
        if ( codeModel()->hasFile( m_file ) ) {
            unsigned int cursorLine, cursorCol;
            m_cursor->cursorPosition(&cursorLine, &cursorCol);

            FunctionDom function = 0;
            FunctionDefinitionDom functionDef = 0;

            FileDom file = codeModel()->fileByName( m_file );

            FunctionList functionList = CodeModelUtils::allFunctions(file);
            FunctionList::ConstIterator theend = functionList.end();
            for( FunctionList::ConstIterator ci = functionList.begin(); ci!= theend; ++ci ){
                int sline, scol;
                int eline, ecol;
                (*ci)->getStartPosition(&sline, &scol);
                (*ci)->getEndPosition(&eline, &ecol);
                if(cursorLine >= sline && cursorLine <= eline )
                    function = *ci;
            }
            if (function == 0){
                FunctionDefinitionList functionDefList = CodeModelUtils::allFunctionDefinitionsDetailed(file).functionList;
                FunctionDefinitionList::ConstIterator theend = functionDefList.end();
                for( FunctionDefinitionList::ConstIterator ci = functionDefList.begin(); ci!= theend; ++ci ){
                    int sline, scol;
                    int eline, ecol;
                    (*ci)->getStartPosition(&sline, &scol);
                    (*ci)->getEndPosition(&eline, &ecol);
                    if(cursorLine >= sline && cursorLine <= eline)
                        functionDef = *ci;
                }
            }

            int line, col;
            if (function != 0)
                function->getStartPosition(&line, &col);
            else if (functionDef != 0)
                functionDef->getStartPosition(&line, &col);
            else
                return;
            QString funcLine = m_activeEditor->textLine(line);
            unsigned int pos = 0;
            unsigned int length = funcLine.length();
            while (pos < length && funcLine.at(pos).isSpace())
                ++pos;
            //store chars used for indenting the line and put it in front of every created doc line
            QString indentChars = funcLine.left(pos);
            QString text = indentChars + "/**\n" + indentChars + " * \n";
            ArgumentList args;
            QString resultType;
            if (function != 0) {
                args = function->argumentList();
                resultType = function->resultType();
            } else {
                args = functionDef->argumentList();
                resultType = functionDef->resultType();
            }
            for( ArgumentList::ConstIterator ci = args.begin(); ci != args.end(); ++ci)
                text += indentChars + " * @param " + (*ci)->name() +" \n";
            if (resultType != "void" && !resultType.isEmpty())
                text += indentChars + " * @return \n";
            text += indentChars + " */\n";
            m_activeEditor->insertText(line, 0, text);
            m_cursor->setCursorPosition( line + 1, indentChars.length() + 3);
        }
    }
}


#include "doxygenpart.moc"
