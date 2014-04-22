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

#include "exceptionsmodel.h"
#include <QSettings>

namespace LeechCraft
{
namespace CertMgr
{
	ExceptionsModel::ExceptionsModel (QSettings& settings, QObject *parent)
	: QStandardItemModel { parent }
	, Settings_ (settings)
	{
	}

	void ExceptionsModel::Populate ()
	{
		auto keys = Settings_.allKeys ();
		std::sort (keys.begin (), keys.end ());

		for (const auto& key : keys)
			Add (key, Settings_.value (key).toBool ());
	}

	void ExceptionsModel::ToggleState (const QModelIndex& index)
	{
		const auto& statusIdx = index.sibling (index.row (), Column::Status);

		const auto newVal = !statusIdx.data (IsAllowed).toBool ();
		const auto item = itemFromIndex (statusIdx);
		item->setText (newVal ?
					tr ("allow") :
					tr ("deny"));
		item->setData (newVal, IsAllowed);

		const auto& keyIndex = index.sibling (index.row (), Column::Name);
		Settings_.setValue (keyIndex.data ().toString (), newVal);
	}

	void ExceptionsModel::Add (const QString& key, bool val)
	{
		QList<QStandardItem*> row
		{
			new QStandardItem { key },
			new QStandardItem { val ? tr ("allow") : tr ("deny") }
		};

		for (auto item : row)
			item->setEditable (false);

		row.at (ExceptionsModel::Status)->setData (val, ExceptionsModel::IsAllowed);

		appendRow (row);
	}
}
}