//----------------------------------------------------------------------------
//    filename             : qextmdichildview.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _QEXTMDICHILDVIEW_H_
#define _QEXTMDICHILDVIEW_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qapplication.h>

#include "qextmdichildfrm.h"

/**
  * @short Base class for all your special view windows.
  * Base class for all interface windows.<br>
  * Defines some virtual functions for later common use.<br>
  * The derived windows 'lives' attached to a QextMdiChildFrm widget<br>
  * managed by QextMdiChildArea, or detached (managed by the window manager.)<br>
  * So remember that the parent() pointer may change , and may be 0 too.<br>
  */

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildView : public QWidget
{
	Q_OBJECT

public:		// Consruction & Destruction
	QextMdiChildView( const QString& caption, QWidget* parentWidget = 0L, const char* name = 0L, WFlags f=0L);
	~QextMdiChildView();
protected:		// Fields
	QString     m_szCaption;
  	QString     m_sTabCaption;
	QWidget*    m_focusedChildWidget;
	QWidget*    m_firstFocusableChildWidget;
	QWidget*    m_lastFocusableChildWidget;
   /** every child view window has an temporary ID in the Window menu of the main frame. */
   int 				m_windowMenuID;

public:     // Methods
	/**
	 * Memorize first focusable child widget <br>
	 */
   void setFirstFocusableChildWidget(QWidget*);
	/**
	 * Memorize last focusable child widget <br>
	 */
   void setLastFocusableChildWidget(QWidget*);
	/**
	 * Returns current focused child widget <br>
	 */
   QWidget* focusedChildWidget();
	/**
	 * Returns TRUE if this window is attached to the Mdi manager<br>
	 * (inline)
	 */
	bool isAttached();
	/**
	 * Returns the caption text<br>
	 * (inline)
	 */
	const QString& caption();
  	const QString& tabCaption();
	/**
	 * Sets the window caption string...<br>
	 * Calls updateButton on the taskbar button if it has been set.<br>
	 */
	virtual void setCaption(const QString& szCaption);

	/** Set the caption of the button referring to this window */
  virtual void setTabCaption(const QString& caption);

  /** Set the caption of both window and button on the taskbar (they are going to be the same) */
  virtual void setMDICaption(const QString &caption);

  // I need this for setting both....
        /**
	 * Returns the QextMdiChildFrm parent widget (or 0 if the window is not attached)
	 */
	QextMdiChildFrm *mdiParent();
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,<br>
	 * or if it is VISIBLE when 'floating'.
	 */
	bool isMinimized();//Useful only when attached (?)
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,<br>
	 * otherwise returns FALSE.
	 */
	bool isMaximized();//Useful only when attached (?)
	/**
	 * Returns the geometry of this window or of the parent if there is any...
	 */
	QRect externalGeometry();
	/**
	 * Sets the geometry of this window or of the parent if there is any...
	 */
	void setExternalGeometry(const QRect& newGeomety);
	//Methods to override ABSOLUTELY
	/**
	 * You SHOULD override this function in the derived class<br>
	 */
	virtual QPixmap * myIconPtr();
	/**
	 * Minimizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
	virtual void minimize(bool bAnimate);   //Useful only when attached
	/**
	 * Maximizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
   virtual void maximize(bool bAnimate);   //Useful only when attached
   /**
     * Interpose in event loop of all current child widgets.
     * Must be recalled after dynamic adding of new child widgets!
     */
   void installEventFilterForAllChildren();
   /**
     * Switches interposing in event loop of all current child widgets off.
     */
   void removeEventFilterForAllChildren();
   /** sets an ID  */
   void setWindowMenuID( int id);
   /** sets the minimum size of the widget to w by h pixels.
     * It extends it base clase method in a way that the minimum size of
     * its childframe (if there is one) will be set, additionally. */
   virtual void setMinimumSize ( int minw, int minh );
   /** sets the maximum size of the widget to w by h pixels.
     * It extends it base clase method in a way that the maximum size of
     * its childframe (if there is one) will be set, additionally. */
   virtual void setMaximumSize ( int minw, int minh );

public slots:
	/**
	 * Attaches this window to the Mdi manager.<br>
	 * It calls the QextMdiMainFrm attachWindow function , so if you have a pointer<br>
	 * to this QextMdiMainFrm you'll be faster calling that function.<br>
	 * Useful as slot.
	 */
	virtual void attach();
	/**
	 * Detaches this window from the Mdi manager.<br>
	 * It calls the QextMdiMainFrm detachWindow function , so if you have a pointer<br>
	 * to this QextMdiMainFrm you'll be faster calling that function.<br>
	 * Useful as slot.
	 */
	virtual void detach();
	
	virtual void minimize(); //Overload and slot
	virtual void maximize(); //Overload and slot
	/**
	 * Restores this window when it is attached to the Mdi manager.
	 */
	virtual void restore();    //Useful only when attached
	virtual void youAreAttached(QextMdiChildFrm *lpC);
	virtual void youAreDetached();
   /** called if someone click on the "Window" menu item for this child frame window */
   virtual void slot_clickedInWindowMenu();
   /** called if someone click on the "Dock/Undock..." menu item for this child frame window */
   virtual void slot_clickedInDockMenu();

protected:	// Protected methods
	/**
	 * Ignores the event and calls QextMdiMainFrm::childWindowCloseRequest
	 * @see QextMdiMainFrm::childWindowCloseRequest
	 */
	virtual void closeEvent(QCloseEvent *e);
   virtual bool eventFilter(QObject *obj, QEvent *e);
   virtual void focusInEvent(QFocusEvent *);
	
signals:
   void attachWindow( QextMdiChildView*,bool);
   void detachWindow( QextMdiChildView*,bool);
   void focusInEventOccurs( QextMdiChildView*);
   void childWindowCloseRequest( QextMdiChildView*);
	
   /** signal emitted when the window caption is changed via setCaption() or setMDICaption() */
   void windowCaptionChanged( const QString&);

   /** signal emitted  when the window caption is changed via setTabCaption() or setMDICaption() */
   void tabCaptionChanged( const QString&);
  
   void mdiParentNowMaximized();
   void mdiParentNoLongerMaximized(QextMdiChildFrm*);
   /** is automatically emitted when slot_clickedInWindowMenu is called */
   void clickedInWindowMenu(int);
   /** is automatically emitted when slot_clickedInDockMenu is called */
   void clickedInDockMenu(int);
};

inline bool QextMdiChildView::isAttached(){ return (parent() != 0); }
// I don't know how to deal with this...
/** Return the caption of the child window (this is differnet from the caption on the button on the taskbar) */
inline const QString& QextMdiChildView::caption(){ return m_szCaption; }
/** Retrun the caption of the button on hte task bar */
inline const QString& QextMdiChildView::tabCaption(){ return m_sTabCaption; }
inline QextMdiChildFrm *QextMdiChildView::mdiParent(){ return (QextMdiChildFrm *)parent(); }

#endif //_QEXTMDICHILDVIEW_H_
