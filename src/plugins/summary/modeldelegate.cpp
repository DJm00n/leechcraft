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

#include "modeldelegate.h"
#include <QApplication>
#include <QStyle>
#include <interfaces/ijobholder.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace Summary
{
	ModelDelegate::ModelDelegate (QObject *parent)
	: QStyledItemDelegate (parent)
	{
	}

	namespace
	{
		bool DrawProgress (QPainter *painter,
				const QStyleOptionViewItem& option, const QModelIndex& index)
		{
			qlonglong done = index.data (ProcessState::Done).toLongLong ();
			qlonglong total = index.data (ProcessState::Total).toLongLong ();
			if (done < 0 || total <= 0)
				return false;

			while (done > 1000 && total > 1000)
			{
				done /= 10;
				total /= 10;
			}

			QStyleOptionProgressBarV2 pbo;
			pbo.rect = option.rect;
			pbo.minimum = 0;
			pbo.maximum = total;
			pbo.progress = done;
			pbo.state = option.state;
			pbo.text = index.data ().toString ();
			pbo.textVisible = true;
			QApplication::style ()->drawControl (QStyle::CE_ProgressBar, &pbo, painter);
			return true;
		}
	}

	void ModelDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.column () == JobHolderColumn::JobProgress)
		{
			auto rowRole = index.data (CustomDataRoles::RoleJobHolderRow).value<JobHolderRow> ();
			if ((rowRole == JobHolderRow::DownloadProgress ||
						rowRole == JobHolderRow::ProcessProgress) &&
					DrawProgress (painter, option, index))
				return;
		}

		QStyledItemDelegate::paint (painter, option, index);
	}
}
}
