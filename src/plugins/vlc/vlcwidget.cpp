/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Vladislav Tyulbashev
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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QToolBar>
#include <QMenu>
#include <QSizePolicy>
#include <QLabel>
#include <QWidgetAction>
#include <QPushButton>
#include <QTimer>
#include <QDebug>
#include <QToolButton>
#include <QUrl>
#include <QEventLoop>
#include "vlcwidget.h"
#include "vlcplayer.h"

namespace LeechCraft
{
namespace vlc
{
	VlcWidget::VlcWidget (QWidget *parent)
	: QWidget (parent)
	, Parent_ (parent)
	, AllowFullScreenPanel_ (false)
	{
		VlcMainWidget_ = new SignalledWidget;
		VlcMainWidget_->SetBackGroundColor (new QColor ("black"));
		QVBoxLayout *layout = new QVBoxLayout;
		layout->setContentsMargins (0, 0, 0, 0);
		layout->addWidget (VlcMainWidget_);
		setLayout (layout);
		VlcPlayer_ = new VlcPlayer (VlcMainWidget_);

		GenerateToolBar ();
		PrepareFullScreen ();
		InterfaceUpdater_ = new QTimer (this);
		InterfaceUpdater_->setInterval (100);
		InterfaceUpdater_->start ();
		
		TerminatePanel_ = new QTimer (this);
		TerminatePanel_->setInterval (3000);
		
		ConnectWidgetToMe (VlcMainWidget_);
		ConnectWidgetToMe (FullScreenWidget_);
		ConnectWidgetToMe (FullScreenPanel_);
		
		connect (TerminatePanel_,
				SIGNAL (timeout ()),
				this,
				SLOT (hideFullScreenPanel ()));
		
		connect (InterfaceUpdater_,
				SIGNAL (timeout ()),
				this,
				SLOT (updateInterface ()));
		
		connect (VlcMainWidget_,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (generateContextMenu (QPoint)));
		
		connect (ScrollBar_,
				SIGNAL (changePosition (double)),
				VlcPlayer_,
				SLOT (changePosition (double)));
		
		connect (Open_,
				SIGNAL (triggered ()),
				this,
				SLOT (addFile ()));
		
		connect (TogglePlay_,
				SIGNAL (triggered ()),
				VlcPlayer_,
				SLOT(togglePlay ()));
		
		connect (Stop_,
				SIGNAL (triggered ()),
				VlcPlayer_,
				SLOT (stop ()));
		
		connect (FullScreenAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (toggleFullScreen ()));
	}
	
	VlcWidget::~VlcWidget()
	{
		VlcPlayer_->stop ();
		delete VlcPlayer_;
		emit deleteMe (this);
	}


	QObject* VlcWidget::ParentMultiTabs ()
	{
		return Parent_;
	}
	
	QToolBar* VlcWidget::GetToolBar () const
	{
		return Bar_;
	}
	
	void VlcWidget::Remove () 
	{	
		deleteLater ();
	}
	
	void VlcWidget::addFile ()
	{
		QString file = QFileDialog::getOpenFileName (this,
													tr ("Open file"),
													tr ("Videos (*.mkv *.avi *.mov *.mpg)"));
		if (QFile::exists (file))
			VlcPlayer_->addUrl (QUrl::fromLocalFile (file));
	}
	
	void VlcWidget::addFolder () 
	{
		QString folder = QFileDialog::getExistingDirectory (this,
													tr ("Open folder"),
													tr ("Folder with video"));
		
		if (QFile::exists (folder))
			VlcPlayer_->addUrl (QUrl ("directory://" + folder));
	}
	
	void VlcWidget::addDVD ()
	{
		QString folder = QFileDialog::getExistingDirectory (this,
													tr ("Open DVD"),
													tr ("Root of DVD directory"));
		
		if (QFile::exists (folder)) 
			VlcPlayer_->addUrl (QUrl ("dvdsimple://" + folder));
	}

	void VlcWidget::addUrl ()
	{
		QString url = QInputDialog::getText (this, tr ("Open URL"), tr ("Enter URL"));
		
		if (!url.isEmpty ())
			VlcPlayer_->addUrl (QUrl (url));
	}
	
	void VlcWidget::updateInterface ()
	{
		if (VlcPlayer_->NowPlaying ()) 
		{
			TogglePlay_->setText (tr ("Pause"));
			TogglePlay_->setProperty ("ActionIcon", "media-playback-pause");
		}
		else
		{
			TogglePlay_->setText (tr ("Play"));
			TogglePlay_->setProperty ("ActionIcon", "media-playback-start");
		}
		
		if (FullScreen_) 
		{
			FullScreenVlcScrollBar_->setPosition (VlcPlayer_->GetPosition ());
			FullScreenVlcScrollBar_->update ();
			FullScreenTimeLeft_->setText (VlcPlayer_->GetCurrentTime ().toString ("HH:mm:ss"));
			FullScreenTimeAll_->setText (VlcPlayer_->GetFullTime ().toString ("HH:mm:ss"));
		}
		else 
		{
			ScrollBar_->setPosition (VlcPlayer_->GetPosition ());
			ScrollBar_->update ();
			TimeLeft_->setText (VlcPlayer_->GetCurrentTime ().toString ("HH:mm:ss"));
			TimeAll_->setText (VlcPlayer_->GetFullTime ().toString ("HH:mm:ss"));
		}
		
		
		if (FullScreen_) 
		{
			if (!FullScreenWidget_->isVisible ())
				toggleFullScreen ();
			else
				FullScreenWidget_->setFocus ();
		}
		else
			VlcMainWidget_->setFocus ();
	}

	void VlcWidget::mouseDoubleClickEvent (QMouseEvent *event)
	{	
		toggleFullScreen ();
		event->accept ();
	}

	void VlcWidget::mousePressEvent (QMouseEvent *event)
	{		
	}
	
	void VlcWidget::wheelEvent (QWheelEvent *event)
	{
		if (event->delta () > 0)
			SoundWidget_->increaseVolume ();
		else
			SoundWidget_->decreaseVolume ();
		
		fullScreenPanelRequested ();
		event->accept ();
	}
	
	void VlcWidget::keyPressEvent (QKeyEvent *event) 
	{
		if (event->text () == tr ("f"))
			toggleFullScreen ();
		
		event->accept ();
	}
	
	void VlcWidget::mouseMoveEvent (QMouseEvent *event)
	{
		if (FullScreen_) 
			fullScreenPanelRequested ();
		
		event->accept ();
	}


	void VlcWidget::toggleFullScreen () 
	{
		if (ForbidFullScreen_)
			return;
				
		ForbidFullScreen ();
		
		if (!FullScreen_)
		{
			FullScreen_ = true;
			AllowFullScreenPanel_ = true;
			FullScreenWidget_->SetBackGroundColor (new QColor ("black"));
			FullScreenWidget_->show ();
			FullScreenWidget_->showFullScreen ();
			VlcPlayer_->switchWidget (FullScreenWidget_);
		} 
		else 
		{
			AllowFullScreenPanel_ = false;
 			FullScreen_ = false;
			FullScreenWidget_->hide ();
			FullScreenPanel_->hide ();
			VlcPlayer_->switchWidget (VlcMainWidget_);
		}
	}
	
	void VlcWidget::GenerateToolBar () 
	{
		Bar_ = new QToolBar (this);
		OpenButton_ = new QToolButton (Bar_);
		OpenButton_->setMenu (GenerateMenuForOpenAction ());
		OpenButton_->setPopupMode (QToolButton::MenuButtonPopup);
		Open_ = new QAction (OpenButton_);
		Open_->setProperty ("ActionIcon", "folder");
		OpenButton_->setDefaultAction (Open_);
		Bar_->addWidget (OpenButton_);
		TogglePlay_ = Bar_->addAction (tr ("Play"));
		TogglePlay_->setShortcut (QKeySequence (" "));
		TogglePlay_->setProperty ("ActionIcon", "media-playback-start");
		TogglePlay_->setProperty ("WatchActionIconChange", true);
		Stop_ = Bar_->addAction (tr ("Stop"));
		Stop_->setProperty ("ActionIcon", "media-playback-stop");
		FullScreenAction_ = Bar_->addAction (tr ("FullScreen"));
		FullScreenAction_->setProperty ("ActionIcon", "view-fullscreen");
		FullScreenAction_->setShortcut (QKeySequence ("f"));
		TimeLeft_ = new QLabel (this);
		Bar_->addWidget (TimeLeft_);
		ScrollBar_ = new VlcScrollBar (this);
		ScrollBar_->setBaseSize (200, 25);
		ScrollBar_->setFocusPolicy (Qt::NoFocus);
		QSizePolicy pol;
		pol.setHorizontalStretch (255);
		pol.setHorizontalPolicy (QSizePolicy::Ignored);
		pol.setVerticalPolicy (QSizePolicy::Expanding);
		ScrollBar_->setSizePolicy(pol);
		Bar_->addWidget (ScrollBar_);
		TimeAll_ = new QLabel;
		TimeAll_->setFocusPolicy (Qt::NoFocus);
		Bar_->addWidget (TimeAll_);
		SoundWidget_ = new SoundWidget (this, VlcPlayer_->GetPlayer ());
		SoundWidget_->setFixedSize (100, 25);
		SoundWidget_->setFocusPolicy (Qt::NoFocus);
		Bar_->addWidget (SoundWidget_);
		Bar_->setFocusPolicy (Qt::NoFocus);
	}
	
	TabClassInfo VlcWidget::GetTabClassInfo () const
	{
		return VlcWidget::GetTabInfo ();
	}
	
	TabClassInfo VlcWidget::GetTabInfo () 
	{
		static TabClassInfo main;
		main.Description_ = "Main tab for VLC plugin";
		main.Priority_ = 1;
		main.Icon_ = QIcon ();
		main.VisibleName_ = "Name of main tab of VLC plugin";
		main.Features_ = TabFeature::TFOpenableByRequest;
		main.TabClass_ = "org.LeechCraft.vlc";
		return main;
	};
	
	void VlcWidget::TabLostCurrent ()
	{
		InterfaceUpdater_->stop ();
	}
	
	void VlcWidget::TabMadeCurrent ()
	{
		InterfaceUpdater_->start ();
	}
	
	void VlcWidget::ForbidFullScreen ()
	{
		ForbidFullScreen_ = true;
		QTimer::singleShot (500, this, SLOT (allowFullScreen ()));
	}
	
	void VlcWidget::allowFullScreen ()
	{
		ForbidFullScreen_ = false;
	}
	
	void VlcWidget::generateContextMenu (QPoint pos)
	{
		ContextMenu_ = new QMenu (this);
		
		QMenu *subtitles = new QMenu (tr ("subtitles"), ContextMenu_);
		QMenu *tracks = new QMenu (tr ("tracks"), ContextMenu_);
		
		for (int i = 0; i < VlcPlayer_->GetAudioTracksNumber (); i++)
		{
			QAction *action = new QAction (tracks);
			action->setData (VlcPlayer_->GetAudioTrackId (i));
			action->setText (VlcPlayer_->GetAudioTrackDescription (i));
			if (VlcPlayer_->GetAudioTrackId (i) == VlcPlayer_->GetCurrentAudioTrack ())
			{
				action->setCheckable (true);
				action->setChecked (true);
			}
			tracks->addAction (action);
		}
		
		for (int i = 0; i < VlcPlayer_->GetSubtitlesNumber (); i++) {
			QAction *action = new QAction (subtitles);
			action->setData (QVariant (VlcPlayer_->GetSubtitleId (i)));
			action->setText (VlcPlayer_->GetSubtitleDescription (i));
			if (VlcPlayer_->GetSubtitleId (i) == VlcPlayer_->GetCurrentSubtitle ())
			{
				action->setCheckable (true);
				action->setChecked (true);
			}
			subtitles->addAction (action);
		}
		
		subtitles->addSeparator ();
		subtitles->addAction (tr ("Add subtitles..."));
		
		ContextMenu_->addMenu (subtitles);
		ContextMenu_->addMenu (tracks);
		
		connect (tracks,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (setAudioTrack (QAction*)));
		
		connect (subtitles,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (setSubtitles (QAction*)));
				
		ContextMenu_->exec (QCursor::pos ());
	}
	
	void VlcWidget::setSubtitles(QAction *action)
	{
		if (action->data ().isNull ())
			VlcPlayer_->AddSubtitles (GetNewSubtitles ());
		else 
		{
			int track = action->data ().toInt ();
			VlcPlayer_->setSubtitle (track);
		}
	}
	
	QString VlcWidget::GetNewSubtitles ()
	{
		return QFileDialog::getOpenFileName (this,
											tr ("Open file"),
											tr ("Subtitles (*.srt)"));
	}
	
	void VlcWidget::setAudioTrack (QAction *action)
	{
		int track = action->data ().toInt ();
		VlcPlayer_->setAudioTrack (track);
	}
	
	void VlcWidget::ConnectWidgetToMe (SignalledWidget *widget)
	{
		connect (widget,
				SIGNAL (mouseDoubleClick (QMouseEvent*)),
				this,
				SLOT (mouseDoubleClickEvent (QMouseEvent*)));
		
		connect (widget,
				SIGNAL (keyPress (QKeyEvent*)),
				this,
				SLOT (keyPressEvent (QKeyEvent*)));
		
		connect (widget,
				SIGNAL (wheel (QWheelEvent*)),
				this,
				SLOT (wheelEvent (QWheelEvent*)));
	}
	
	void VlcWidget::PrepareFullScreen ()
	{
		FullScreen_ = false;
		ForbidFullScreen_ = false;
		FullScreenWidget_ = new SignalledWidget;
		FullScreenWidget_->addAction (TogglePlay_);
		FullScreenPanel_ = new SignalledWidget (FullScreenWidget_, Qt::Widget | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		FullScreenPanel_->SetBackGroundColor (new QColor (palette ().button ().color ()));
		QHBoxLayout *panelLayout = new QHBoxLayout;
		FullScreenTimeLeft_ = new QLabel;
		FullScreenTimeAll_ = new QLabel;
		FullScreenVlcScrollBar_ = new VlcScrollBar (this);
		FullScreenSoundWidget_ = new SoundWidget (this, VlcPlayer_->GetPlayer ());
		FullScreenSoundWidget_->setFixedSize (100, 25);
		QSizePolicy fullScreenVlcScrollBarPolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
		fullScreenVlcScrollBarPolicy.setHorizontalStretch (255);
		FullScreenVlcScrollBar_->setSizePolicy (fullScreenVlcScrollBarPolicy);
		
		TogglePlayButton_ = new QToolButton;
		TogglePlayButton_->setDefaultAction (TogglePlay_);
		TogglePlayButton_->setAutoRaise (true);
		StopButton_ = new QToolButton;
		StopButton_->setDefaultAction (Stop_);
		StopButton_->setAutoRaise (true);
		FullScreenButton_ = new QToolButton;
		FullScreenButton_->setDefaultAction (FullScreenAction_);
		FullScreenButton_->setAutoRaise (true);
		
		panelLayout->addWidget (TogglePlayButton_);
		panelLayout->addWidget (StopButton_);
		panelLayout->addWidget (FullScreenButton_);
		panelLayout->addWidget (FullScreenTimeLeft_);
		panelLayout->addWidget (FullScreenVlcScrollBar_);
		panelLayout->addWidget (FullScreenTimeAll_);
		panelLayout->addWidget (FullScreenSoundWidget_);
		panelLayout->setContentsMargins (5, 0, 5, 0);
		
		FullScreenPanel_->setLayout (panelLayout);
		FullScreenPanel_->setWindowOpacity (0.8);
		
		FullScreenTimeLeft_->show ();
		FullScreenVlcScrollBar_->show ();
		FullScreenWidget_->setMouseTracking (true);
		FullScreenPanel_->setMouseTracking (true);

		
		connect (FullScreenWidget_,
				SIGNAL (mouseMove (QMouseEvent*)),
				this,
				SLOT (mouseMoveEvent (QMouseEvent*)));
		
		connect (FullScreenPanel_,
				SIGNAL (mouseMove (QMouseEvent*)),
				this,
				SLOT (mouseMoveEvent (QMouseEvent*)));
		
		connect (FullScreenVlcScrollBar_,
				SIGNAL (changePosition (double)),
				VlcPlayer_,
				SLOT (changePosition (double)));
		
		connect (FullScreenWidget_,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (generateContextMenu (QPoint)));
		
		connect (FullScreenPanel_,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (generateContextMenu (QPoint)));
		
		connect (FullScreenWidget_,
				SIGNAL(resize (QResizeEvent*)),
				this,
				SLOT(fullScreenPanelRequested ()));
	}
	
	void VlcWidget::fullScreenPanelRequested ()
	{
		if (!AllowFullScreenPanel_ || !FullScreenWidget_->isVisible ())
			return;
		
		FullScreenPanel_->setGeometry (5, FullScreenWidget_->height () - 30, FullScreenWidget_->width () - 10, 25);
		if (!FullScreenPanel_->isVisible ())
			FullScreenPanel_->show ();
		else
			FullScreenPanel_->update ();
		
		TerminatePanel_->start ();
	}
	
	void VlcWidget::hideFullScreenPanel ()
	{
		TerminatePanel_->stop ();
		ForbidPanel ();
		FullScreenWidget_->setFocus ();
		FullScreenPanel_->hide ();
	}
	
	void VlcWidget::AllowPanel ()
	{
		AllowFullScreenPanel_ = true;
	}
	
	void VlcWidget::ForbidPanel ()
	{
		AllowFullScreenPanel_ = false;
		QTimer::singleShot (50, this, SLOT (AllowPanel ()));
	}
	
	QMenu* VlcWidget::GenerateMenuForOpenAction ()
	{
		QMenu *result = new QMenu;
		connect (result->addAction (tr ("Open folder")),
				SIGNAL (triggered ()),
				this,
				SLOT (addFolder ()));
		
		connect (result->addAction (tr ("Open url")),
				SIGNAL (triggered ()),
				this,
				SLOT (addUrl ()));
		
		connect (result->addAction (tr ("Open DVD")),
				SIGNAL (triggered ()),
				this,
				SLOT (addDVD ()));
		
		return result;
	}
}
}
