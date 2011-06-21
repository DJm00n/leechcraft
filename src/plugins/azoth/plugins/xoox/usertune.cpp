/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "usertune.h"
#include <QDomElement>
#include <QXmppElement.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsTuneNode = "http://jabber.org/protocol/tune";
	
	QString UserTune::GetNodeString ()
	{
		return NsTuneNode;
	}
	
	namespace
	{
		void AppendTxt (QXmppElement& tune, const QString& tag, const QString& str)
		{
			if (str.isEmpty ())
				return;
			
			QXmppElement elem;
			elem.setTagName (tag);
			elem.setValue (str);
			tune.appendChild (elem);
		}
	}
	
	QXmppElement UserTune::ToXML () const
	{
		QXmppElement result;
		result.setTagName ("item");
		
		QXmppElement tune;
		tune.setTagName ("tune");
		tune.setAttribute ("xmlns", NsTuneNode);
		
		AppendTxt (tune, "artist", Artist_);
		AppendTxt (tune, "source", Source_);
		AppendTxt (tune, "title", Title_);
		AppendTxt (tune, "track", Track_);
		AppendTxt (tune, "uri", URI_.toEncoded ());
		if (Length_)
			AppendTxt (tune, "length", QString::number (Length_));
		if (Rating_)
			AppendTxt (tune, "rating", QString::number (Rating_));
		
		return result;
	}
	
	void UserTune::Parse (const QDomElement& elem)
	{
		QDomElement tune = elem.firstChildElement ("tune");
		if (tune.namespaceURI () != NsTuneNode)
			return;
		
		Artist_ = tune.firstChildElement ("artist").text ();
		Source_ = tune.firstChildElement ("source").text ();
		Title_ = tune.firstChildElement ("title").text ();
		Track_ = tune.firstChildElement ("track").text ();
		URI_ = QUrl::fromEncoded (tune.firstChildElement ("uri").text ().toUtf8 ());
		Length_ = tune.firstChildElement ("length").text ().toInt ();
		Rating_ = tune.firstChildElement ("rating").text ().toInt ();
	}
	
	PEPEventBase* UserTune::Clone () const
	{
		return new UserTune (*this);
	}
	
	QString UserTune::GetArtist () const
	{
		return Artist_;
	}
	
	void UserTune::SetArtist (const QString& artist)
	{
		Artist_ = artist;
	}
	
	QString UserTune::GetSource () const
	{
		return Source_;
	}

	void UserTune::SetSource (const QString& source)
	{
		Source_ = source;
	}

	QString UserTune::GetTitle () const
	{
		return Title_;
	}

	void UserTune::SetTitle (const QString& title)
	{
		Title_ = title;
	}

	QString UserTune::GetTrack () const
	{
		return Track_;
	}

	void UserTune::SetTrack (const QString& track)
	{
		Track_ = track;
	}

	QUrl UserTune::GetURI () const
	{
		return URI_;
	}

	void UserTune::SetURI (const QUrl& uri)
	{
		URI_ = uri;
	}
	
	int UserTune::GetLength () const
	{
		return Length_;
	}
	
	void UserTune::SetLength (int length)
	{
		Length_ = length;
	}
	
	int UserTune::GetRating () const
	{
		return Rating_;
	}
	
	void UserTune::SetRating (int rating)
	{
		Rating_ = rating;
	}
}
}
}
