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

#include "bioviewmanager.h"
#include <numeric>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <util/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/sys/paths.h>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include "biopropproxy.h"
#include "core.h"
#include "util.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class DiscoModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				AlbumName = Qt::UserRole + 1,
				AlbumYear,
				AlbumImage,
				AlbumTrackListTooltip
			};

			DiscoModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::AlbumName] = "albumName";
				roleNames [Roles::AlbumYear] = "albumYear";
				roleNames [Roles::AlbumImage] = "albumImage";
				roleNames [Roles::AlbumTrackListTooltip] = "albumTrackListTooltip";
				setRoleNames (roleNames);
			}
		};

		const int AASize = 170;
	}

	BioViewManager::BioViewManager (QDeclarativeView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, BioPropProxy_ (new BioPropProxy (this))
	, DiscoModel_ (new DiscoModel (this))
	{
		auto proxy = Core::Instance ().GetProxy ();
		View_->rootContext ()->setContextObject (BioPropProxy_);
		View_->rootContext ()->setContextProperty ("artistDiscoModel", DiscoModel_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		View_->engine ()->addImageProvider ("sysIcons", new Util::ThemeImageProvider (proxy));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);
	}

	void BioViewManager::InitWithSource ()
	{
		connect (View_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (View_->rootObject (),
				SIGNAL (albumPreviewRequested (int)),
				this,
				SLOT (handleAlbumPreviewRequested (int)));
	}

	void BioViewManager::Request (Media::IArtistBioFetcher *fetcher, const QString& artist)
	{
		DiscoModel_->clear ();

		CurrentArtist_ = artist;

		auto pending = fetcher->RequestArtistBio (CurrentArtist_);
		connect (pending->GetQObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleBioReady ()));

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<Media::IDiscographyProvider*> ())
		{
			auto fetcher = prov->GetDiscography (CurrentArtist_);
			connect (fetcher->GetQObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handleDiscographyReady ()));
		}
	}

	QStandardItem* BioViewManager::FindAlbumItem (const QString& albumName) const
	{
		for (int i = 0, rc = DiscoModel_->rowCount (); i < rc; ++i)
		{
			auto item = DiscoModel_->item (i);
			const auto& itemData = item->data (DiscoModel::Roles::AlbumName);
			if (itemData.toString () == albumName)
				return item;
		}
		return 0;
	}

	void BioViewManager::SetAlbumImage (const QString& album, const QImage& img)
	{
		auto item = FindAlbumItem (album);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item for"
					<< album;
			return;
		}

		item->setData (Util::GetAsBase64Src (img), DiscoModel::Roles::AlbumImage);
	}

	void BioViewManager::handleBioReady ()
	{
		auto pending = qobject_cast<Media::IPendingArtistBio*> (sender ());
		const auto& bio = pending->GetArtistBio ();
		BioPropProxy_->SetBio (bio);

		emit gotArtistImage (bio.BasicInfo_.Name_, bio.BasicInfo_.LargeImage_);
	}

	void BioViewManager::handleDiscographyReady ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		auto aaProvObj = pm->GetAllCastableRoots<Media::IAlbumArtProvider*> ().value (0);
		auto aaProv = qobject_cast<Media::IAlbumArtProvider*> (aaProvObj);
		if (aaProvObj)
			connect (aaProvObj,
					SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)),
					this,
					SLOT (handleAlbumArt (Media::AlbumInfo, QList<QImage>)),
					Qt::UniqueConnection);

		auto fetcher = qobject_cast<Media::IPendingDisco*> (sender ());
		const auto& icon = Core::Instance ().GetProxy ()->GetIcon ("media-optical").pixmap (AASize * 2, AASize * 2);
		for (const auto& release : fetcher->GetReleases ())
		{
			if (FindAlbumItem (release.Name_))
				continue;

			auto item = new QStandardItem;
			item->setData (release.Name_, DiscoModel::Roles::AlbumName);
			item->setData (QString::number (release.Year_), DiscoModel::Roles::AlbumYear);
			item->setData (Util::GetAsBase64Src (icon.toImage ()), DiscoModel::Roles::AlbumImage);

			item->setData (MakeTrackListTooltip (release.TrackInfos_),
					DiscoModel::Roles::AlbumTrackListTooltip);

			auto tracks = std::accumulate (release.TrackInfos_.begin (), release.TrackInfos_.end (),
					decltype (release.TrackInfos_.value (0)) ());
			Album2Tracks_ << tracks;

			DiscoModel_->appendRow (item);

			aaProv->RequestAlbumArt ({ CurrentArtist_, release.Name_ });
		}
	}

	namespace
	{
		struct ScaleResult
		{
			QImage Image_;
			QString Album_;
		};
	}

	void BioViewManager::handleAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		if (info.Artist_ != CurrentArtist_ || images.isEmpty ())
			return;

		auto img = images.first ();
		if (img.width () <= AASize)
		{
			SetAlbumImage (info.Album_, img);
			return;
		}

		auto watcher = new QFutureWatcher<ScaleResult> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleImageScaled ()));

		watcher->setFuture (QtConcurrent::run ([img, info] () -> ScaleResult
				{ return { img.scaled (AASize, AASize, Qt::KeepAspectRatio, Qt::SmoothTransformation), info.Album_ }; }));
	}

	void BioViewManager::handleImageScaled ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<ScaleResult>*> (sender ());
		watcher->deleteLater ();

		const auto& result = watcher->result ();
		SetAlbumImage (result.Album_, result.Image_);
	}

	void BioViewManager::handleAlbumPreviewRequested (int index)
	{
		QList<QPair<QString, int>> tracks;
		for (const auto& track : Album2Tracks_.at (index))
			tracks.push_back ({ track.Name_, track.Length_ });

		const auto& album = DiscoModel_->item (index)->data (DiscoModel::Roles::AlbumName).toString ();

		auto ph = Core::Instance ().GetPreviewHandler ();
		ph->previewAlbum (CurrentArtist_, album, tracks);
	}

	void BioViewManager::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
