/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "readitlateraccount.h"
#include <QDataStream>
#include <QtDebug>
#include <util/util.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	ReadItLaterAccount::ReadItLaterAccount (const QString& login, QObject *parent)
	: QObject (parent)
	, Login_ (login)
	, AuthType_ (IAccount::ATHttpAuth)
	, ParentService_ (parent)
	, IsSyncing_ (false)
	, LastUpload_ (QDateTime::currentDateTime ())
	, LastDownload_ (QDateTime::currentDateTime ())
	{
	}

	QObject* ReadItLaterAccount::GetObject ()
	{
		return this;
	}

	QObject* ReadItLaterAccount::GetParentService () const
	{
		return ParentService_;
	}

	QByteArray ReadItLaterAccount::GetAccountID () const
	{
		return QString ("org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater.%1")
				.arg (Login_).toUtf8 ();
	}

	QString ReadItLaterAccount::GetLogin () const
	{
		return Login_;
	}

	QString ReadItLaterAccount::GetPassword () const
	{
		return Password_;
	}

	void ReadItLaterAccount::SetPassword (const QString& pass)
	{
		Password_ = pass;
	}

	IAccount::AuthType ReadItLaterAccount::GetAuthType () const
	{
		return AuthType_;
	}

	QVariantMap ReadItLaterAccount::GetIdentifyingData () const
	{
		QVariantMap map;
		map ["Login"] = Login_;
		map ["Password_"] = Password_;
		return map;
	}

	bool ReadItLaterAccount::IsSyncing () const
	{
		return IsSyncing_;
	}

	void ReadItLaterAccount::SetSyncing (bool sync)
	{
		IsSyncing_ = sync;
	}

	QDateTime ReadItLaterAccount::GetLastDownloadDateTime () const
	{
		return LastDownload_;
	}

	void ReadItLaterAccount::SetLastDownloadDateTime (const QDateTime& dt)
	{
		LastDownload_ = dt;
	}

	QDateTime ReadItLaterAccount::GetLastUploadDateTime () const
	{
		return LastUpload_;
	}

	void ReadItLaterAccount::SetLastUploadDateTime (const QDateTime& dt)
	{
		LastUpload_ = dt;
	}

	QByteArray ReadItLaterAccount::Serialize () const
	{
		quint16 version = 1;

		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << version
					<< Login_
					<< IsSyncing_
					<< LastUpload_
					<< LastDownload_;
		}

		return result;
	}

	ReadItLaterAccount* ReadItLaterAccount::Deserialize (const QByteArray& data,
			QObject *parent)
	{
		quint16 version = 0;

		QDataStream in (data);
		in >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return 0;
		}

		QString login;
		bool isSyncing;
		in >> login
			>> isSyncing;
		ReadItLaterAccount *acc = new ReadItLaterAccount (login, parent);
		acc->SetSyncing (isSyncing);
		in >> acc->LastUpload_
			>> acc->LastDownload_;
		return acc;
	}

}
}
}
}
