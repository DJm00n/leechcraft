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

#include "commands.h"
#include <boost/range/adaptor/reversed.hpp>
#include <QStringList>
#include <QtDebug>
#include <QUrl>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/isupportnonroster.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/core/ientitymanager.h>

namespace LeechCraft
{
namespace Azoth
{
namespace MuCommands
{
	namespace
	{
		void InjectMessage (IProxyObject *azothProxy, ICLEntry *entry, const QString& contents)
		{
			const auto entryObj = entry->GetQObject ();
			const auto msgObj = azothProxy->CreateCoreMessage (contents,
					QDateTime::currentDateTime (),
					IMessage::MTServiceMessage,
					IMessage::DIn,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();
		}
	}

	bool HandleNames (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());

		QStringList names;
		for (const auto obj : mucEntry->GetParticipants ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< obj
						<< "doesn't implement ICLEntry";
				continue;
			}
			const QString& name = entry->GetEntryName ();
			if (!name.isEmpty ())
				names << name;
		}
		names.sort ();

		const auto& contents = QObject::tr ("MUC's participants: ") + "<ul><li>" +
				names.join ("</li><li>") + "</li></ul>";
		InjectMessage (azothProxy, entry, contents);

		return true;
	}

	namespace
	{
		QStringList GetAllUrls (IProxyObject *azothProxy, ICLEntry *entry)
		{
			QStringList urls;
			for (const auto msgObj : entry->GetAllMessages ())
			{
				const auto msg = qobject_cast<IMessage*> (msgObj);
				switch (msg->GetMessageType ())
				{
				case IMessage::MTChatMessage:
				case IMessage::MTMUCMessage:
					break;
				default:
					continue;
				}

				urls += azothProxy->FindLinks (msg->GetBody ());
			}
			return urls;
		}
	}

	bool ListUrls (IProxyObject *azothProxy, ICLEntry *entry, const QString&)
	{
		const auto& urls = GetAllUrls (azothProxy, entry);

		const auto& body = urls.isEmpty () ?
				QObject::tr ("Sorry, no links found, chat more!") :
				QObject::tr ("Found links:") + "<ol><li>" + urls.join ("</li><li>") + "</li></ol>";
		InjectMessage (azothProxy, entry, body);

		return true;
	}

	bool OpenUrl (const ICoreProxy_ptr& coreProxy, IProxyObject *azothProxy,
			ICLEntry *entry, const QString& text, TaskParameters params)
	{
		const auto& urls = GetAllUrls (azothProxy, entry);

		const auto& split = text.split (' ', QString::SkipEmptyParts).mid (1);

		QList<int> indexes;
		if (split.isEmpty ())
			indexes << urls.size () - 1;

		for (const auto& item : split)
		{
			bool ok = false;
			const auto idx = item.toInt (&ok);
			if (ok)
				indexes << idx - 1;
		}

		const auto iem = coreProxy->GetEntityManager ();
		for (const auto idx : indexes)
		{
			const auto& url = urls.value (idx);
			if (url.isEmpty ())
				continue;

			const auto& entity = Util::MakeEntity (QUrl::fromUserInput (url),
					{}, params | FromUserInitiated);
			iem->HandleEntity (entity);
		}

		return true;
	}

	namespace
	{
		QHash<QString, ICLEntry*> GetParticipants (IMUCEntry *entry)
		{
			QHash<QString, ICLEntry*> result;
			for (const auto entryObj : entry->GetParticipants ())
			{
				const auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (entry)
					result [entry->GetEntryName ()] = entry;
			}
			return result;
		}

		QStringList ParseNicks (ICLEntry *entry, const QString& text)
		{
			auto split = text
					.section (' ', 1)
					.split ('\n', QString::SkipEmptyParts);

			if (!split.isEmpty ())
				return split;

			const auto& msgs = entry->GetAllMessages ();
			for (const auto msgObj : boost::adaptors::reverse (msgs))
			{
				const auto msg = qobject_cast<IMessage*> (msgObj);
				if (const auto otherPart = qobject_cast<ICLEntry*> (msg->OtherPart ()))
				{
					split << otherPart->GetEntryName ();
					break;
				}
			}

			return split;
		}

		ICLEntry* ResolveEntry (const QString& name, const QHash<QString, ICLEntry*>& context, QObject *accObj)
		{
			if (context.contains (name))
				return context.value (name);

			const auto acc = qobject_cast<IAccount*> (accObj);
			for (const auto entryObj : acc->GetCLEntries ())
			{
				const auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry)
					continue;

				if (entry->GetEntryName () == name || entry->GetHumanReadableID () == name)
					return entry;
			}

			if (const auto isn = qobject_cast<ISupportNonRoster*> (accObj))
				if (const auto entry = qobject_cast<ICLEntry*> (isn->CreateNonRosterItem (name)))
					return entry;

			return nullptr;
		}

		QString FormatRepresentation (const QList<QPair<QString, QVariant>>& repr)
		{
			QStringList strings;

			for (const auto& pair : repr)
			{
				if (pair.second.isNull ())
					continue;

				auto string = "<strong>" + pair.first + ":</strong> ";

				switch (pair.second.type ())
				{
				case QVariant::String:
				{
					const auto& metaStr = pair.second.toString ();
					if (metaStr.isEmpty ())
						continue;

					string += metaStr;
					break;
				}
				case QVariant::Image:
				{
					const auto& image = pair.second.value<QImage> ();
					if (image.isNull ())
						continue;

					const auto& src = Util::GetAsBase64Src (image);
					string += "<img src='" + src + "' alt=''/>";
					break;
				}
				case QVariant::Date:
				{
					const auto& date = pair.second.toDate ();
					if (date.isNull ())
						continue;

					string += date.toString (Qt::DefaultLocaleLongDate);
					break;
				}
				case QVariant::StringList:
				{
					const auto& list = pair.second.toStringList ();
					if (list.isEmpty ())
						continue;

					string += "<ul><li>" + list.join ("</li><li>") + "</li></ul>";
					break;
				}
				default:
					string += "unhandled data type ";
					string += pair.second.typeName ();
					break;
				}

				strings << string;
			}

			if (strings.isEmpty ())
				return {};

			return "<ul><li>" + strings.join ("</li><li>") + "</li></ul>";
		}
	}

	bool ShowVCard (IProxyObject *azothProxy, ICLEntry *entry, const QString& text)
	{
		const auto& split = ParseNicks (entry, text);
		if (split.isEmpty ())
			return true;

		const auto& participants = GetParticipants (qobject_cast<IMUCEntry*> (entry->GetQObject ()));
		for (const auto& name : split)
		{
			const auto target = ResolveEntry (name.trimmed (),
					participants, entry->GetParentAccount ());
			if (!target)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("Unable to resolve %1.").arg ("<em>" + name + "</em>"));
				continue;
			}

			const auto targetObj = target->GetQObject ();
			const auto imie = qobject_cast<IMetaInfoEntry*> (targetObj);
			if (!imie)
			{
				InjectMessage (azothProxy, entry,
						QObject::tr ("%1 doesn't support extended metainformation.").arg ("<em>" + name + "</em>"));
				continue;
			}

			const auto& repr = FormatRepresentation (imie->GetVCardRepresentation ());
			if (repr.isEmpty ())
			{
				InjectMessage (azothProxy, entry,
						name + ": " + QObject::tr ("no information, would wait for next vcard update..."));

				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[azothProxy, entry, text] { ShowVCard (azothProxy, entry, text); },
					targetObj,
					SIGNAL (vcardUpdated ()),
					targetObj
				};
			}
			else
				InjectMessage (azothProxy, entry, name + ":<br/>" + repr);
		}

		return true;
	}
}
}
}
