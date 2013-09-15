/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "mwproxy.h"
#include <QDockWidget>
#include "core.h"
#include "mainwindow.h"
#include "dockmanager.h"

namespace LeechCraft
{
	MWProxy::MWProxy (MainWindow *win, QObject *parent)
	: QObject (parent)
	, Win_ (win)
	{
	}

	void MWProxy::AddDockWidget (Qt::DockWidgetArea area, QDockWidget *w)
	{
		Core::Instance ().GetDockManager ()->AddDockWidget (w, area);
		ToggleViewActionVisiblity (w, true);
	}

	void MWProxy::AssociateDockWidget (QDockWidget *dock, QWidget *tab)
	{
		Core::Instance ().GetDockManager ()->AssociateDockWidget (dock, tab);
	}

	void MWProxy::SetDockWidgetVisibility (QDockWidget *dock, bool visible)
	{
		Core::Instance ().GetDockManager ()->SetDockWidgetVisibility (dock, visible);
	}

	void MWProxy::ToggleViewActionVisiblity (QDockWidget *w, bool visible)
	{
		Core::Instance ().GetDockManager ()->ToggleViewActionVisiblity (w, visible);
	}

	void MWProxy::SetViewActionShortcut (QDockWidget *w, const QKeySequence& seq)
	{
		w->toggleViewAction ()->setShortcut (seq);
	}

	void MWProxy::AddToolbar (QToolBar *bar, Qt::ToolBarArea area)
	{
		bar->setParent (Win_);
		Win_->addToolBar (area, bar);
	}

	void MWProxy::AddSideWidget (QWidget *w, WidgetArea area)
	{
		auto splitter = Win_->GetMainSplitter ();

		switch (area)
		{
		case WALeft:
			splitter->insertWidget (0, w);
			break;
		case WARight:
			splitter->addWidget (w);
			break;
		case WABottom:
			qWarning () << Q_FUNC_INFO
					<< "not implemented yet";
			break;
		}
	}

	void MWProxy::ToggleVisibility ()
	{
		Win_->showHideMain ();
	}

	QMenu* MWProxy::GetMainMenu ()
	{
		return Win_->GetMainMenu ();
	}

	void MWProxy::HideMainMenu ()
	{
		Win_->HideMainMenu ();
	}
}
