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
#include <stdexcept>
#include <QObject>
#include <QHash>
#include <QVariantList>
#include <QVariantMap>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iclentry.h>
#include "structures.h"

class QTimer;

namespace LeechCraft
{
namespace Util
{
	class QueueManager;

	namespace SvcAuth
	{
		class VkAuthManager;
	}
}

namespace Azoth
{
namespace Murm
{
	class LongPollManager;
	class Logger;

	class VkConnection : public QObject
	{
		Q_OBJECT

		LeechCraft::Util::SvcAuth::VkAuthManager * const AuthMgr_;
		const ICoreProxy_ptr Proxy_;

		Logger& Logger_;

		QByteArray LastCookies_;
	public:
		typedef QMap<QString, QString> UrlParams_t;

		class PreparedCall_f
		{
			std::function<QNetworkReply* (QString, UrlParams_t)> Call_;

			UrlParams_t Params_;
		public:
			PreparedCall_f () = default;

			template<typename T>
			PreparedCall_f (T c)
			: Call_ (c)
			{
			}

			QNetworkReply* operator() (const QString& key) const
			{
				return Call_ (key, Params_);
			}

			void ClearParams ()
			{
				Params_.clear ();
			}

			void AddParam (const QPair<QString, QString>& pair)
			{
				Params_ [pair.first] = pair.second;
			}
		};

		class CommandException : public std::runtime_error
		{
		public:
			CommandException (const QString&);
		};

		class RecoverableException : public CommandException
		{
		public:
			RecoverableException ();
		};

		class UnrecoverableException : public CommandException
		{
			const int Code_;
			const QString Msg_;
		public:
			UnrecoverableException (int, const QString&);

			int GetCode () const;
			const QString& GetMessage () const;
		};
	private:
		QList<PreparedCall_f> PreparedCalls_;
		LeechCraft::Util::QueueManager *CallQueue_;

		typedef QList<QPair<QNetworkReply*, PreparedCall_f>> RunningCalls_t;
		RunningCalls_t RunningCalls_;

		EntryStatus Status_;
		EntryStatus CurrentStatus_;

		LongPollManager *LPManager_;
	public:
		typedef std::function<void (QHash<int, QString>)> GeoSetter_f;
		typedef std::function<void (QList<PhotoInfo>)> PhotoInfoSetter_f;
		typedef std::function<void (FullMessageInfo)> MessageInfoSetter_f;
	private:
		QHash<int, std::function<void (QVariantList)>> Dispatcher_;
		QHash<QNetworkReply*, std::function<void (qulonglong)>> MsgReply2Setter_;
		QHash<QNetworkReply*, GeoSetter_f> CountryReply2Setter_;

		QHash<QNetworkReply*, MessageInfoSetter_f> Reply2MessageSetter_;
		QHash<QNetworkReply*, PhotoInfoSetter_f> Reply2PhotoSetter_;

		QHash<QNetworkReply*, QString> Reply2ListName_;

		QHash<QNetworkReply*, ChatInfo> Reply2ChatInfo_;

		struct ChatRemoveInfo
		{
			qulonglong Chat_;
			qulonglong User_;
		};
		QHash<QNetworkReply*, ChatRemoveInfo> Reply2ChatRemoveInfo_;

		QHash<QString, PreparedCall_f> CaptchaId2Call_;

		int APIErrorCount_ = 0;
		bool ShouldRerunPrepared_ = false;

		bool MarkingOnline_ = false;
		QTimer * const MarkOnlineTimer_;
	public:
		enum class Type
		{
			Dialog,
			Chat
		};

		VkConnection (const QString&, const QByteArray&, ICoreProxy_ptr, Logger&);

		const QByteArray& GetCookies () const;

		void RerequestFriends ();

		void SendMessage (qulonglong to,
				const QString& body,
				std::function<void (qulonglong)> idSetter,
				Type type,
				const QStringList& attachments);
		void SendTyping (qulonglong to);
		void MarkAsRead (const QList<qulonglong>&);
		void RequestGeoIds (const QList<int>&, GeoSetter_f, GeoIdType);

		void GetUserInfo (const QList<qulonglong>& ids);

		void GetMessageInfo (qulonglong id, MessageInfoSetter_f setter);
		void GetPhotoInfos (const QStringList& ids, PhotoInfoSetter_f setter);

		void AddFriendList (const QString&, const QList<qulonglong>&);
		void ModifyFriendList (const ListInfo&, const QList<qulonglong>&);

		void SetNRIList (const QList<qulonglong>&);

		void CreateChat (const QString&, const QList<qulonglong>&);
		void RequestChatInfo (qulonglong);
		void AddChatUser (qulonglong chat, qulonglong user);
		void RemoveChatUser (qulonglong chat, qulonglong user);
		void SetChatTitle (qulonglong, const QString&);

		void SetStatus (QString);

		void SetStatus (const EntryStatus&, bool updateString);
		EntryStatus GetStatus () const;

		void SetMarkingOnlineEnabled (bool);

		void QueueRequest (PreparedCall_f);
		static void AddParams (QUrl&, const UrlParams_t&);

		void HandleCaptcha (const QString& cid, const QString& value);

		bool Q_REQUIRED_RESULT CheckFinishedReply (QNetworkReply*);
		void CheckReplyData (const QVariant&, QNetworkReply*);
	private:
		void HandleMessage (const QVariantList&);

		void PushFriendsRequest ();

		RunningCalls_t::const_iterator FindRunning (QNetworkReply*) const;
		RunningCalls_t::iterator FindRunning (QNetworkReply*);

		void RescheduleRequest (QNetworkReply*);
	public slots:
		void reauth ();
	private slots:
		void rerunPrepared ();
		void callWithKey (const QString&);

		void handleReplyDestroyed ();

		void markOnline ();

		void handleListening ();
		void handlePollError ();
		void handlePollStopped ();
		void handlePollData (const QVariantMap&);

		void handleFriendListAdded ();
		void handleGotSelfInfo ();
		void handleGotFriendLists ();
		void handleGotFriends ();
		void handleGotNRI ();
		void handleGotUserInfo ();
		void handleGotUnreadMessages ();

		void handleChatCreated ();
		void handleChatInfo ();
		void handleChatUserRemoved ();

		void handleMessageSent ();
		void handleCountriesFetched ();
		void handleMessageInfoFetched ();
		void handlePhotoInfosFetched ();

		void handleScopeSettingsChanged ();

		void saveCookies (const QByteArray&);
	signals:
		void statusChanged (EntryStatus);

		void cookiesChanged ();

		void stoppedPolling ();

		void gotSelfInfo (const UserInfo&);

		void gotLists (const QList<ListInfo>&);
		void addedLists (const QList<ListInfo>&);
		void gotUsers (const QList<UserInfo>&);
		void gotNRIList (const QList<qulonglong>&);
		void gotMessage (const MessageInfo&);
		void gotTypingNotification (qulonglong uid);

		void gotChatInfo (const ChatInfo&);
		void chatUserRemoved (qulonglong, qulonglong);

		void userStateChanged (qulonglong uid, bool online);

		void mucChanged (qulonglong);

		void captchaNeeded (const QString& sid, const QUrl& url);
	};
}
}
}
