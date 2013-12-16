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

#include "structuresops.h"

QDataStream& operator<< (QDataStream& out, const LeechCraft::Entity& e)
{
	quint16 version = 2;
	out << version
		<< e.Entity_
		<< e.Location_
		<< e.Mime_
		<< static_cast<quint32> (e.Parameters_)
		<< e.Additional_;
	return out;
}

QDataStream& operator>> (QDataStream& in, LeechCraft::Entity& e)
{
	quint16 version = 0;
	in >> version;
	if (version == 2)
	{
		quint32 parameters;
		in >> e.Entity_
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		if (parameters & LeechCraft::NoAutostart)
			e.Parameters_ |= LeechCraft::NoAutostart;
		if (parameters & LeechCraft::DoNotSaveInHistory)
			e.Parameters_ |= LeechCraft::DoNotSaveInHistory;
		if (parameters & LeechCraft::IsDownloaded)
			e.Parameters_ |= LeechCraft::IsDownloaded;
		if (parameters & LeechCraft::FromUserInitiated)
			e.Parameters_ |= LeechCraft::FromUserInitiated;
		if (parameters & LeechCraft::DoNotNotifyUser)
			e.Parameters_ |= LeechCraft::DoNotNotifyUser;
		if (parameters & LeechCraft::Internal)
			e.Parameters_ |= LeechCraft::Internal;
		if (parameters & LeechCraft::NotPersistent)
			e.Parameters_ |= LeechCraft::NotPersistent;
		if (parameters & LeechCraft::DoNotAnnounceEntity)
			e.Parameters_ |= LeechCraft::DoNotAnnounceEntity;
		if (parameters & LeechCraft::OnlyHandle)
			e.Parameters_ |= LeechCraft::OnlyHandle;
		if (parameters & LeechCraft::OnlyDownload)
			e.Parameters_ |= LeechCraft::OnlyDownload;
		if (parameters & LeechCraft::AutoAccept)
			e.Parameters_ |= LeechCraft::AutoAccept;
		if (parameters & LeechCraft::ShouldQuerySource)
			e.Parameters_ |= LeechCraft::ShouldQuerySource;
	}
	else if (version == 1)
	{
		QByteArray buf;
		quint32 parameters;
		in >> buf
			>> e.Location_
			>> e.Mime_
			>> parameters
			>> e.Additional_;

		e.Entity_ = buf;

		if (parameters & LeechCraft::NoAutostart)
			e.Parameters_ |= LeechCraft::NoAutostart;
		if (parameters & LeechCraft::DoNotSaveInHistory)
			e.Parameters_ |= LeechCraft::DoNotSaveInHistory;
		if (parameters & LeechCraft::IsDownloaded)
			e.Parameters_ |= LeechCraft::IsDownloaded;
		if (parameters & LeechCraft::FromUserInitiated)
			e.Parameters_ |= LeechCraft::FromUserInitiated;
		if (parameters & LeechCraft::DoNotNotifyUser)
			e.Parameters_ |= LeechCraft::DoNotNotifyUser;
		if (parameters & LeechCraft::Internal)
			e.Parameters_ |= LeechCraft::Internal;
		if (parameters & LeechCraft::NotPersistent)
			e.Parameters_ |= LeechCraft::NotPersistent;
		if (parameters & LeechCraft::DoNotAnnounceEntity)
			e.Parameters_ |= LeechCraft::DoNotAnnounceEntity;
		if (parameters & LeechCraft::OnlyHandle)
			e.Parameters_ |= LeechCraft::OnlyHandle;
		if (parameters & LeechCraft::OnlyDownload)
			e.Parameters_ |= LeechCraft::OnlyDownload;
		if (parameters & LeechCraft::AutoAccept)
			e.Parameters_ |= LeechCraft::AutoAccept;
		if (parameters & LeechCraft::ShouldQuerySource)
			e.Parameters_ |= LeechCraft::ShouldQuerySource;
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< "version";
	}
	return in;
}

namespace LeechCraft
{
	bool operator< (const LeechCraft::Entity& e1, const LeechCraft::Entity& e2)
	{
		return e1.Mime_ < e2.Mime_ &&
			e1.Location_ < e2.Location_ &&
			e1.Parameters_ < e2.Parameters_;
	}

	bool operator== (const LeechCraft::Entity& e1, const LeechCraft::Entity& e2)
	{
		return e1.Mime_ == e2.Mime_ &&
			e1.Entity_ == e2.Entity_ &&
			e1.Location_ == e2.Location_ &&
			e1.Parameters_ == e2.Parameters_ &&
			e1.Additional_ == e2.Additional_;
	}
}
