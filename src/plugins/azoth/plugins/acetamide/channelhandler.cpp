/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "channelhandler.h"
#include "channelclentry.h"
#include "channelpublicmessage.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircserverhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ChannelHandler::ChannelHandler (IrcServerHandler *ish, const
			ChannelOptions& channel)
	: ISH_ (ish)
	, ChannelOptions_ (channel)
	, ChannelID_ (channel.ChannelName_ + "@" + channel.ServerName_)
	{
		ChannelCLEntry_ = new ChannelCLEntry (this);
	}

	QString ChannelHandler::GetChannelID () const
	{
		return ChannelID_;
	}

	ChannelCLEntry* ChannelHandler::GetCLEntry () const
	{
		return ChannelCLEntry_;
	}

	IrcServerHandler* ChannelHandler::GetIrcServerHandler () const
	{
		return ISH_;
	}

	ChannelOptions ChannelHandler::GetChannelOptions () const
	{
		return ChannelOptions_;
	}

	QList<QObject*> ChannelHandler::GetParticipants () const
	{
		QList<QObject*> result;
		Q_FOREACH (ServerParticipantEntry_ptr spe,
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
			result << spe.get ();
		return result;
	}

	ServerParticipantEntry_ptr ChannelHandler::GetSelf ()
	{
		Q_FOREACH (ServerParticipantEntry_ptr spe,
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
		{
			if (spe->GetEntryName () == ISH_->GetNickName ())
				return spe;
		}

		return ServerParticipantEntry_ptr ();
	}

	IrcMessage* ChannelHandler::CreateMessage (IMessage::MessageType t,
			const QString& variant, const QString& body)
	{
		IrcMessage *msg = new IrcMessage (t,
				IMessage::DIn,
				variant,
				ISH_->GetNickName (),
				ISH_->GetAccount ()->GetClientConnection ().get ());
		msg->SetBody (body);
		msg->SetDateTime (QDateTime::currentDateTime ());

		return msg;
	}

	void ChannelHandler::SendPublicMessage (const QString& msg)
	{
		if (msg.startsWith ('/'))
		{
			ISH_->ParseMessageForCommand (msg, ChannelID_);
			return;
		}

		ISH_->SendPublicMessage (msg, ChannelID_);

		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (ISH_->GetNickName ());

		if (!entry)
			return;

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
						IMessage::DIn,
						ChannelCLEntry_,
						IMessage::MTMUCMessage,
						IMessage::MSTOther,
						entry);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::HandleIncomingMessage (const QString& nick,
			const QString& msg)
	{
		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (nick);

		if (!entry)
			return;

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
						IMessage::DIn,
						ChannelCLEntry_,
						IMessage::MTMUCMessage,
						IMessage::MSTOther,
						entry);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetChannelUser (const QString& nick)
	{
		QString nickname = nick;
		ChannelRole role;
		switch (nick [0].toAscii ())
		{
			case '~':
				role = Owner;
				break;
			case '&':
				role = Admin;
				break;
			case '@':
				role = Operator;
				break;
			case '%':
				role = HalfOperator;
				break;
			case '+':
				role = Voiced;
				break;
			default:
				role = Participant;
		}

		if (role != Participant)
			nickname = nickname.mid (1);

		ServerParticipantEntry_ptr entry = ISH_->
				GetParticipantEntry (nickname);
		QStringList groups = entry->GetChannels ();
		if (!groups.contains (ChannelOptions_.ChannelName_))
		{
			groups << ChannelOptions_.ChannelName_;
			entry->SetGroups (groups);
			entry->SetRole (ChannelOptions_.ChannelName_, role);
			MakeJoinMessage (nickname);
			entry->SetStatus (EntryStatus (SOnline, QString ()));
		}
	}

	void ChannelHandler::RemoveChannelUser (const QString& nick,
			const QString& msg)
	{
		ServerParticipantEntry_ptr entry =
				ISH_->GetParticipantEntry (nick);
		if (!entry)
			return;

		QStringList groups = entry->GetChannels ();
		if (groups.contains (ChannelOptions_.ChannelName_))
		{
			if (groups.removeOne (ChannelOptions_.ChannelName_))
			{
				MakeLeaveMessage (nick, msg);
				ISH_->GetAccount ()->handleEntryRemoved (entry.get ());
				if (!groups.count () && !entry->IsPrivateChat ())
					ISH_->RemoveParticipantEntry (nick);
				else
					ISH_->GetParticipantEntry (nick)->SetGroups (groups);
			}
		}
	}

	void ChannelHandler::MakeJoinMessage (const QString& nick)
	{
		QString msg  = tr ("%1 joined the channel").arg (nick);

		ChannelPublicMessage *message =
				new ChannelPublicMessage (msg,
					IMessage::DIn,
					ChannelCLEntry_,
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantJoin);
		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::MakeLeaveMessage (const QString& nick,
			const QString& msg)
	{
		QString mess;
		if (!msg.isEmpty ())
			mess = tr ("%1 has left the channel (%2)").arg (nick, msg);
		else
			mess = tr ("%1 has left the channel").arg (nick);

		ChannelPublicMessage *message =
				new ChannelPublicMessage (mess,
					IMessage::DIn,
					ChannelCLEntry_,
					IMessage::MTStatusMessage,
					IMessage::MSTParticipantLeave);

		ChannelCLEntry_->HandleMessage (message);
	}

	void ChannelHandler::SetMUCSubject (const QString& subject)
	{
		Subject_ = subject;

		ChannelPublicMessage *message =
				new ChannelPublicMessage (subject,
							IMessage::DIn,
							ChannelCLEntry_,
							IMessage::MTEventMessage,
							IMessage::MSTRoomSubjectChange);
		ChannelCLEntry_->HandleMessage (message);
	}

	QString ChannelHandler::GetMUCSubject () const
	{
		return Subject_;
	}

	void ChannelHandler::LeaveChannel (const QString& msg)
	{
		Q_FOREACH (ServerParticipantEntry_ptr entry,
				ISH_->GetParticipants (ChannelOptions_.ChannelName_))
		{
			QStringList list = entry->GetChannels ();
			bool prChat = entry->IsPrivateChat ();

			if (list.contains (ChannelOptions_.ChannelName_))
			{
				list.removeAll (ChannelOptions_.ChannelName_);
				if (!list.count () && !prChat)
				{
					ISH_->GetAccount ()->
							handleEntryRemoved (entry.get ());
					ISH_->RemoveParticipantEntry (entry->
							GetEntryName ());
				}
				else
					entry->SetGroups (list);
			}
		}

		ISH_->LeaveChannel (ChannelOptions_.ChannelName_, msg);
		RemoveThis ();
	}

	void ChannelHandler::RemoveThis ()
	{
		ISH_->GetAccount ()->handleEntryRemoved (ChannelCLEntry_);

		ISH_->UnregisterChannel (this);

		deleteLater ();
	}

};
};
};
