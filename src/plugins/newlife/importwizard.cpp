/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "importwizard.h"
#include <QtDebug>
#include "importers/akregator/akregatorimporter.h"
#include "importers/liferea/lifereaimporter.h"
#include "importers/ktorrent/ktorrentimporter.h"
#include "importers/firefox/firefoximporter.h"

namespace LeechCraft
{
namespace NewLife
{
	ImportWizard::ImportWizard (QWidget *parent)
	: QWizard (parent)
	{
		Ui_.setupUi (this);

		Importers_ << new Importers::AkregatorImporter (this);
		Importers_ << new Importers::LifereaImporter (this);
		Importers_ << new Importers::KTorrentImporter (this);
		Importers_ << new Importers::FirefoxImporter (this);

		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()),
				Qt::QueuedConnection);
		connect (this,
				SIGNAL (accepted ()),
				this,
				SLOT (handleRejected ()),
				Qt::QueuedConnection);

		SetupImporters ();
	}

	QString ImportWizard::GetSelectedName () const
	{
		return Ui_.FirstPage_->GetSelectedName ();
	}

	void ImportWizard::handleAccepted ()
	{
		deleteLater ();
	}

	void ImportWizard::handleRejected ()
	{
		deleteLater ();
	}

	void ImportWizard::SetupImporters ()
	{
		Q_FOREACH (AbstractImporter *ai, Importers_)
			Ui_.FirstPage_->SetupImporter (ai);
	}
}
}
