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

#include "eventssettingsmanager.h"
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <xmlsettingsdialog/datasourceroles.h>
#include <util/xpc/stdanfields.h>
#include <interfaces/an/constants.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Tracolor
{
	EventsSettingsManager::EventsSettingsManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Event"), tr ("Color") });
		Model_->setHeaderData (0, Qt::Horizontal,
				DataSources::DataFieldType::None,
				DataSources::DataSourceRole::FieldType);
		Model_->setHeaderData (1, Qt::Horizontal,
				DataSources::DataFieldType::Color,
				DataSources::DataSourceRole::FieldType);
		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));

		LoadSettings ();
	}

	QMap<QString, EventsSettingsManager::EventInfo> EventsSettingsManager::GetEnabledEvents () const
	{
		return EnabledEvents_;
	}

	QAbstractItemModel* EventsSettingsManager::GetModel () const
	{
		return Model_;
	}

	void EventsSettingsManager::AppendRow (const QString& eventId,
			const QColor& color, bool isEnabled)
	{
		const QList<QStandardItem*> row
		{
			new QStandardItem { eventId },
			new QStandardItem { color.name () }
		};
		for (auto item : row)
			item->setEditable (false);
		row.first ()->setCheckable (true);
		row.first ()->setCheckState (isEnabled ? Qt::Checked : Qt::Unchecked);
		row.value (1)->setForeground (color);
		Model_->appendRow (row);
	}

	void EventsSettingsManager::LoadSettings ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Tracolor" };
		if (!settings.childGroups ().contains ("Events"))
		{
			LoadDefaultSettings ();
			return;
		}

		const auto eventsCount = settings.beginReadArray ("Events");
		for (int i = 0; i < eventsCount; ++i)
		{
			settings.setArrayIndex (i);

			const auto isEnabled = settings.value ("IsEnabled").toBool ();
			const auto& event = settings.value ("Event").toString ();
			const auto& color = settings.value ("Color").toString ();

			AppendRow (event, QColor { color }, isEnabled);
		}
		settings.endArray ();

		RebuildEnabledEvents ();
	}

	void EventsSettingsManager::LoadDefaultSettings ()
	{
		AppendRow (AN::TypeIMMUCMsg, "green");
		AppendRow (AN::TypeIMIncMsg, "magenta");
		AppendRow (AN::TypeIMMUCHighlight, "red");

		RebuildEnabledEvents ();
	}

	void EventsSettingsManager::RebuildEnabledEvents ()
	{
		EnabledEvents_.clear ();

		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			const auto firstItem = Model_->item (i, 0);
			if (firstItem->checkState () != Qt::Checked)
				continue;

			const auto& id = firstItem->text ();
			const auto& color = Model_->item (i, 1)->text ();
			EnabledEvents_ [id] = EventInfo { color };
		}

		emit eventsSettingsChanged ();
	}

	void EventsSettingsManager::modifyRequested (const QString&, int rowIdx, const QVariantList& datas)
	{
		const auto colorItem = Model_->item (rowIdx, 1);
		const auto& color = datas.value (0).value<QColor> ();

		disconnect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));
		colorItem->setText (color.name ());
		colorItem->setForeground (color);
		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));

		saveSettings ();
		RebuildEnabledEvents ();
	}

	void EventsSettingsManager::saveSettings ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Tracolor" };
		settings.beginWriteArray ("Events");
		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			settings.setArrayIndex (i);

			settings.setValue ("IsEnabled", Model_->item (i, 0)->checkState () == Qt::Checked);
			settings.setValue ("Event", Model_->item (i, 0)->text ());
			settings.setValue ("Color", Model_->item (i, 1)->text ());
		}
		settings.endArray ();
	}

	void EventsSettingsManager::handleItemChanged ()
	{
		saveSettings ();
		RebuildEnabledEvents ();
	}
}
}
}