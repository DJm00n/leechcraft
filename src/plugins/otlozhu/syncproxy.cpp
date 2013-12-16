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

#include "syncproxy.h"
#include <QtDebug>
#include <laretz/operation.h>
#include <laretz/opsummer.h>
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"
#include "stager.h"

namespace LeechCraft
{
namespace Otlozhu
{
	SyncProxy::SyncProxy (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* SyncProxy::GetQObject ()
	{
		return this;
	}

	namespace
	{
		std::string ToStdStr (const QString& str)
		{
			return str.toUtf8 ().constData ();
		}
	}

	QList<Laretz::Operation> SyncProxy::GetAllOps () const
	{
		Core::Instance ().GetStager ()->Enable ();

		std::vector<Laretz::Item> items;

		const auto todoMgr = Core::Instance ().GetTodoManager ();
		for (const auto& todoItem : todoMgr->GetTodoStorage ()->GetAllItems ())
		{
			Laretz::Item it (ToStdStr (todoItem->GetID ()), 0);
			Util::Sync::FillItem (it, todoItem->ToMap ());
			items.push_back (std::move (it));
		}

		return { { Laretz::OpType::Append, items } };
	}

	QList<Laretz::Operation> SyncProxy::GetNewOps () const
	{
		auto stager = Core::Instance ().GetStager ();
		return stager->IsEnabled () ? stager->GetStagedOps () : GetAllOps ();
	}

	void SyncProxy::Merge (QList<Laretz::Operation>&, const QList<Laretz::Operation>& theirs)
	{
		auto guard = Core::Instance ().GetStager ()->EnterMergeMode ();

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		for (const auto& op : theirs)
		{
			const auto& items = op.getItems ();
			switch (op.getType ())
			{
			case Laretz::OpType::Fetch:
				for (const auto& item : items)
				{
					const auto pos = storage->FindItem (QString::fromUtf8 (item.getId ().c_str ()));
					if (pos == -1)
					{
						TodoItem_ptr todo (new TodoItem (QString::fromUtf8 (item.getId ().c_str ())));
						todo->ApplyDiff (Util::Sync::ItemToMap (item));
						storage->AddItem (todo);
					}
					else
					{
						auto todo = storage->GetItemAt (pos);
						todo->ApplyDiff (Util::Sync::ItemToMap (item));
						storage->HandleUpdated (todo);
					}
				}
				break;
			case Laretz::OpType::Delete:
				for (const auto& item : items)
					storage->RemoveItem (QString::fromUtf8 (item.getId ().c_str ()));
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown operation type"
						<< static_cast<int> (op.getType ());
				break;
			}
		}
	}
}
}
