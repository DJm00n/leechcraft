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

#pragma once

#include <functional>
#include <QObject>
#include <QHash>
#include <QVariantMap>
#include <interfaces/core/icoreproxy.h>

class QStandardItem;

namespace LeechCraft
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

class QueueManager;
}

namespace TouchStreams
{
	class AlbumsManager;
	class RecsManager;

	class FriendsManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		Util::QueueManager * const Queue_;
		QList<std::function<void (QString)>> RequestQueue_;
		const std::shared_ptr<void> RequestQueueGuard_;

		QHash<qulonglong, QStandardItem*> Friend2Item_;
		QHash<qulonglong, std::shared_ptr<AlbumsManager>> Friend2AlbumsManager_;
		QHash<qulonglong, std::shared_ptr<RecsManager>> Friend2RecsManager_;

		QStandardItem *Root_;

		QHash<QNetworkReply*, QMap<qlonglong, QVariantMap>> Reply2Users_;

		typedef std::function<QNetworkReply* (QMap<QString, QString>)> ReplyMaker_f;
		QHash<QNetworkReply*, ReplyMaker_f> Reply2Func_;
		ReplyMaker_f CaptchaReplyMaker_;
	public:
		FriendsManager (Util::SvcAuth::VkAuthManager*, Util::QueueManager*, ICoreProxy_ptr, QObject* = 0);

		QStandardItem* GetRootItem () const;

		void RefreshItems (QList<QStandardItem*>);
	private:
		void ScheduleTracksRequests (const QList<qlonglong>&, const QMap<qlonglong, QVariantMap>&);
		void ShowFriendsList (const QList<qlonglong>&, const QMap<qlonglong, QVariantMap>&);
		void MakeFriendItem (qlonglong id, const QVariantMap& userInfo,
				const QVariant& albums, const QVariant& tracks);
	private slots:
		void refetchFriends ();
		void handleGotFriends ();

		void handleCaptchaEntered (const QString&, const QString&);
		void handleExecuted ();

		void handleAlbumsFinished (AlbumsManager*);
		void handlePhotoFetched ();
	};
}
}
