/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "recsmanager.h"
#include <QStandardItem>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QtDebug>
#include <util/svcauth/vkauthmanager.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/media/iradiostationprovider.h>

namespace LeechCraft
{
namespace TouchStreams
{
	RecsManager::RecsManager (boost::optional<qulonglong> uid,
			Util::SvcAuth::VkAuthManager *authMgr,
			Util::QueueManager *queueMgr,
			const ICoreProxy_ptr& proxy,
			QObject *parent)
	: QObject { parent }
	, UID_ { uid }
	, AuthMgr_ { authMgr }
	, QueueMgr_ { queueMgr }
	, Proxy_ { proxy }
	, RootItem_ { new QStandardItem { tr ("VKontakte: recommendations") } }
	{
		RootItem_->setIcon (QIcon (":/touchstreams/resources/images/vk.svg"));
		RootItem_->setEditable (false);

		AuthMgr_->ManageQueue (&RequestQueue_);

		if (!UID_)
		{
			if (AuthMgr_->HadAuthentication ())
				QTimer::singleShot (1000,
						this,
						SLOT (refetchRecs ()));
			connect (AuthMgr_,
					SIGNAL (justAuthenticated ()),
					this,
					SLOT (refetchRecs ()));
		}
	}

	QStandardItem* RecsManager::GetRootItem () const
	{
		return RootItem_;
	}

	QStandardItem* RecsManager::RefreshItems (const QList<QStandardItem*>&)
	{
		refetchRecs ();
		return RootItem_;
	}

	void RecsManager::refetchRecs ()
	{
		RequestQueue_.append ({
				[this] (const QString& key) -> void
				{
					QUrl url ("https://api.vk.com/method/audio.getRecommendations");
					Util::UrlOperator { url }
							("access_token", key)
							("shuffle", "0")
							("count", "100");
					if (UID_)
						Util::UrlOperator { url }
								("user_id", QString::number (*UID_));

					auto nam = Proxy_->GetNetworkAccessManager ();
					connect (nam->get (QNetworkRequest { url }),
							SIGNAL (finished ()),
							this,
							SLOT (handleRecsFetched ()));
				},
				Util::QueuePriority::Normal
			});
		AuthMgr_->GetAuthKey ();
	}

	void RecsManager::handleRecsFetched ()
	{
		if (const auto rc = RootItem_->rowCount ())
			RootItem_->removeRows (0, rc);

		const auto reply = qobject_cast<QNetworkReply*> (sender ());

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();;
		reply->deleteLater ();

		for (const auto& songVar : data ["response"].toList ())
		{
			const auto& map = songVar.toMap ();

			const auto& url = QUrl::fromEncoded (map ["url"].toString ().toUtf8 ());
			if (!url.isValid ())
				continue;

			Media::AudioInfo info {};
			info.Title_ = map ["title"].toString ();
			info.Artist_ = map ["artist"].toString ();
			info.Length_ = map ["duration"].toInt ();
			info.Other_ ["URL"] = url;

			auto trackItem = new QStandardItem (QString::fromUtf8 ("%1 — %2")
						.arg (info.Artist_)
						.arg (info.Title_));
			trackItem->setEditable (false);
			trackItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
			trackItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
					Media::RadioItemRole::TracksInfos);
			RootItem_->appendRow (trackItem);
		}
	}
}
}
