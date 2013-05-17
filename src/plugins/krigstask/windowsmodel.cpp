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

#include "windowsmodel.h"
#include <QtDebug>
#include <QIcon>
#include "xwrapper.h"

namespace LeechCraft
{
namespace Krigstask
{
	WindowsModel::WindowsModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
		XWrapper w;
		auto windows = w.GetWindows ();

		for (auto wid : windows)
			Windows_.append ({ wid, w.GetWindowTitle (wid), w.GetWindowIcon (wid) });
	}

	int WindowsModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	int WindowsModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Windows_.size ();
	}

	QModelIndex WindowsModel::index (int row, int column, const QModelIndex& parent) const
	{
		return hasIndex (row, column, parent) ?
				createIndex (row, column) :
				QModelIndex {};
	}

	QModelIndex WindowsModel::parent (const QModelIndex&) const
	{
		return {};
	}

	QVariant WindowsModel::data (const QModelIndex& index, int role) const
	{
		const auto row = index.row ();
		const auto& item = Windows_.at (row);

		switch (role)
		{
		case Qt::DisplayRole:
			return item.Title_;
		case Qt::DecorationRole:
			return item.Icon_;
		}

		return {};
	}
}
}
