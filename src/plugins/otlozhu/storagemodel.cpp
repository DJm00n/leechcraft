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

#include "storagemodel.h"
#include <QApplication>
#include <QPalette>
#include <QtDebug>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "todostorage.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Otlozhu
{
	StorageModel::StorageModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Storage_ (0)
	{
		Headers_ << tr ("Title")
				<< tr ("Tags")
				<< tr ("Due date")
				<< tr ("Created")
				<< tr ("Percentage");
	}

	QVariant StorageModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
			return QVariant ();
		return Headers_ [section];
	}

	QModelIndex StorageModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!parent.isValid ())
			return createIndex (row, column);

		if (parent.parent ().isValid ())
			return {};

		return createIndex (row, column, parent.row ());
	}

	QModelIndex StorageModel::parent (const QModelIndex& index) const
	{
		if (!index.internalId ())
			return {};

		return createIndex (index.internalId (), 0);
	}

	int StorageModel::columnCount (const QModelIndex&) const
	{
		return Columns::MAX;
	}

	int StorageModel::rowCount (const QModelIndex& parent) const
	{
		if (!parent.isValid ())
			return Storage_ ? Storage_->GetNumItems () : 0;

		if (parent.parent ().isValid ())
			return 0;

		const auto& item = GetItemForIndex (parent);
		return item ? item->GetDeps ().size () : 0;
	}

	Qt::ItemFlags StorageModel::flags (const QModelIndex& index) const
	{
		return QAbstractItemModel::flags (index) | Qt::ItemIsEditable;
	}

	namespace
	{
		QString MakeTooltip (const TodoItem_ptr item)
		{
			QString result = "<strong>" + item->GetTitle () + "</strong><br />";
			result += StorageModel::tr ("%1% done").arg (item->GetPercentage ()) + "<br />";
			const auto& ids = item->GetTagIDs ();
			if (!ids.isEmpty ())
				result += Core::Instance ().GetProxy ()->
						GetTagsManager ()->JoinIDs (ids) + "<br />";

			const QString& comment = item->GetComment ();
			if (!comment.isEmpty ())
				result += comment;

			return result;
		}
	}

	QVariant StorageModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto& item = GetItemForIndex (index);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item for index"
					<< index
					<< index.parent ();
			return {};
		}

		switch (role)
		{
		case Roles::ItemID:
			return item->GetID ();
		case Roles::ItemTitle:
			return item->GetTitle ();
		case Roles::ItemTags:
			return item->GetTagIDs ();
		case Roles::ItemProgress:
			return item->GetPercentage ();
		case Roles::ItemComment:
			return item->GetComment ();
		case Roles::ItemDueDate:
			return item->GetDueDate ();
		case Qt::ToolTipRole:
			return MakeTooltip (item);
		case Qt::DisplayRole:
		case Qt::EditRole:
			break;
		case Qt::ForegroundRole:
		{
			if (item->GetPercentage () != 100 ||
					!XmlSettingsManager::Instance ().property ("DoneGreyOut").toBool ())
				return {};

			auto brush = QApplication::palette ().foreground ();
			brush.setColor (brush.color ().lighter ());
			return brush;
		}
		default:
			return {};
		}

		switch (index.column ())
		{
		case Columns::Title:
			return item->GetTitle ();
		case Columns::Tags:
		{
			const auto& ids = item->GetTagIDs ();
			if (ids.isEmpty ())
				return QString ();
			return Core::Instance ().GetProxy ()->GetTagsManager ()->JoinIDs (ids);
		}
		case Columns::DueDate:
		{
			const auto& date = item->GetDueDate ();
			if (role == Qt::DisplayRole)
				return date.isNull () ? QVariant (tr ("not set")) : QVariant (date);
			else
				return date.isNull () ? QDateTime::currentDateTime () : date;
		}
		case Columns::Created:
			return item->GetCreatedDate ();
		case Columns::Percentage:
		{
			if (const auto perc = item->GetPercentage ())
				return perc;

			const auto& deps = item->GetDeps ();
			if (deps.isEmpty ())
				return 0;

			const auto res = std::accumulate (deps.begin (), deps.end (), 0.0,
					[this] (double val, const QString& id)
					{
						return val + Storage_->GetItemByID (id)->GetPercentage ();
					});
			return std::round (res / deps.size ());
		}
		default:
			return QVariant ();
		}
	}

	bool StorageModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid ())
			return false;

		auto item = Storage_->GetItemByID (index.data (Roles::ItemID).toString ());
		bool updated = false;

		if (role == Roles::ItemProgress)
		{
			item->SetPercentage (value.toInt ());
			updated = true;
		}
		else if (role == Roles::ItemComment)
		{
			item->SetComment (value.toString ());
			updated = true;
		}
		else if (role == Roles::ItemDueDate)
		{
			item->SetDueDate (value.toDateTime ());
			updated = true;
		}
		else if (role == Qt::EditRole)
			switch (index.column ())
			{
			case Columns::Title:
				item->SetTitle (value.toString ());
				updated = true;
				break;
			case Columns::Percentage:
				item->SetPercentage (value.toInt ());
				updated = true;
				break;
			case Columns::DueDate:
				item->SetDueDate (value.toDateTime ());
				updated = true;
				break;
			case Columns::Tags:
			{
				auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
				item->SetTagIDs (tm->SplitToIDs (value.toString ()));
				updated = true;
				break;
			}
			default:
				qDebug () << Q_FUNC_INFO << index.column () << value;
				break;
			}

		if (updated)
			Storage_->HandleUpdated (item);

		return updated;
	}

	TodoItem_ptr StorageModel::GetItemForIndex (const QModelIndex& index) const
	{
		const auto& parent = index.parent ();
		if (!parent.isValid ())
			return Storage_->GetItemAt (index.row ());

		const auto& parentId = data (parent, Roles::ItemID).toString ();
		const auto& parentItem = Storage_->GetItemByID (parentId);
		if (!parentItem)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get item for parent ID"
					<< parentId;
			return {};
		}

		const auto& itemId = parentItem->GetDeps ().value (index.row ());
		if (itemId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get dep ID for dep"
					<< index.row ()
					<< parentItem->GetDeps ();
			return {};
		}

		return Storage_->GetItemByID (itemId);
	}

	void StorageModel::SetStorage (TodoStorage *storage)
	{
		if (Storage_)
			disconnect (Storage_,
					0,
					this,
					0);
		Storage_ = storage;
		if (Storage_)
		{
			connect (Storage_,
					SIGNAL (itemAdded (int)),
					this,
					SLOT (handleItemAdded (int)));
			connect (Storage_,
					SIGNAL (itemUpdated (int)),
					this,
					SLOT (handleItemUpdated (int)));
			connect (Storage_,
					SIGNAL (itemRemoved (int)),
					this,
					SLOT (handleItemRemoved (int)));

			connect (Storage_,
					SIGNAL (itemDepAdded (int, int)),
					this,
					SLOT (handleItemDepAdded (int, int)));
			connect (Storage_,
					SIGNAL (itemDepRemoved (int, int)),
					this,
					SLOT (handleItemDepRemoved (int, int)));
		}

		reset ();
	}

	void StorageModel::handleItemAdded (int idx)
	{
		beginInsertRows ({}, idx, idx);
		endInsertRows ();
	}

	void StorageModel::handleItemUpdated (int idx)
	{
		emit dataChanged (index (idx, 0), index (idx, Columns::MAX - 1));
	}

	void StorageModel::handleItemRemoved (int idx)
	{
		beginRemoveRows ({}, idx, idx);
		endRemoveRows ();
	}

	void StorageModel::handleItemDepAdded (int idx, int depIdx)
	{
		const auto& parent = index (idx, 0);
		beginInsertRows (parent, depIdx, depIdx);
		endInsertRows ();
	}

	void StorageModel::handleItemDepRemoved (int idx, int depIdx)
	{
		const auto& parent = index (idx, 0);
		beginRemoveRows (parent, depIdx, depIdx);
		endRemoveRows ();
	}
}
}
