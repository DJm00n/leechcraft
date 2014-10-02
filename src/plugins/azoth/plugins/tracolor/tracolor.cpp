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

#include "tracolor.h"
#include <QIcon>
#include <QtDebug>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/azoth/iclentry.h>
#include "entryeventsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Tracolor
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		EventsManager_ = new EntryEventsManager;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Tracolor";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Tracolor";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Indicates contacts activity via color coding.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		const bool can = e.Mime_.startsWith ("x-leechcraft/notification") &&
				e.Additional_ [AN::EF::EventCategory].toString () != AN::CatEventCancel &&
				e.Additional_.contains ("org.LC.Plugins.Azoth.SourceID");
		return can ?
				EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
				EntityTestHandleResult {};
	}

	void Plugin::Handle (Entity e)
	{
		const auto& sourceId = e.Additional_ ["org.LC.Plugins.Azoth.SourceID"].toString ();
		const auto& eventId = e.Additional_ [AN::EF::EventType].toString ();

		EventsManager_->HandleEvent (sourceId.toUtf8 (), eventId.toUtf8 ());

		if (e.Additional_.contains ("org.LC.Plugins.Azoth.SubSourceID"))
		{
			const auto& subId = e.Additional_ ["org.LC.Plugins.Azoth.SubSourceID"].toString ();
			EventsManager_->HandleEvent (subId.toUtf8 (), eventId.toUtf8 ());
		}
	}

	void Plugin::hookCollectContactIcons (IHookProxy_ptr,
			QObject *entryObj, QList<QIcon>& icons) const
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& sourceId = entry->GetEntryID ().toUtf8 ();
		const auto value = EventsManager_->GetEntryEventRate (sourceId, AN::TypeIMMUCMsg.toUtf8 ());

		if (!value)
			return;

		QPixmap px { 22, 22 };
		QColor color { Qt::red };
		color.setAlphaF (value);
		px.fill (color);

		icons.prepend ({ px });
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_tracolor, LeechCraft::Azoth::Tracolor::Plugin);