/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "uploadmanager.h"
#include <QNetworkAccessManager>
#include <QFileInfo>
#include "account.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	UploadManager::UploadManager (const QString& path,
			UploadType ut, Account *account)
	: QObject (account)
	, Account_ (account)
	, FilePath_ (path)
	, NAM_ (new QNetworkAccessManager (this))
	, UploadType_ (ut)
	{
		connect (this,
				SIGNAL (finished ()),
				this,
				SLOT (deleteLater ()),
				Qt::QueuedConnection);

		connect (Account_->GetDriveManager (),
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));

		connect (Account_->GetDriveManager (),
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));


		if (UploadType_ == UploadType::Upload)
			InitiateUploadSession ();
	}

	void UploadManager::InitiateUploadSession ()
	{
		Account_->GetDriveManager ()->Upload (FilePath_);
	}

	void UploadManager::handleUploadProgress (qint64 sent, qint64 total)
	{
		//TODO show upload in summary
	}

	void UploadManager::handleFinished ()
	{
		//TODO notify about upload
		emit finished ();
	}

}
}
}
