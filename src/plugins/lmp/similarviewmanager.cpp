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

#include "similarviewmanager.h"
#include <algorithm>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <util/qml/colorthemeproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/ipendingsimilarartists.h>
#include "similarmodel.h"
#include "core.h"
#include "stdartistactionsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	SimilarViewManager::SimilarViewManager (QDeclarativeView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Model_ (new SimilarModel (this))
	{
		View_->rootContext ()->setContextProperty ("similarModel", Model_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ().GetProxy ()->GetColorThemeManager (), this));
	}

	void SimilarViewManager::InitWithSource ()
	{
		new StdArtistActionsManager (View_, this);
	}

	void SimilarViewManager::DefaultRequest (const QString& artist)
	{
		auto similars = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<Media::ISimilarArtists*> ();

		for (auto *similar : similars)
		{
			auto obj = similar->GetSimilarArtists (artist, 20);
			if (!obj)
				continue;

			connect (obj->GetQObject (),
					SIGNAL (error ()),
					obj->GetQObject (),
					SLOT (deleteLater ()));
			connect (obj->GetQObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handleSimilarReady ()));
		}
	}

	void SimilarViewManager::SetInfos (Media::SimilarityInfos_t infos)
	{
		Model_->clear ();

		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo& left, const Media::SimilarityInfo& right)
					{ return left.Similarity_ > right.Similarity_; });

		for (const auto& info : infos)
		{
			auto item = SimilarModel::ConstructItem (info.Artist_);

			QString simStr;
			if (info.Similarity_ > 0)
				simStr = tr ("Similarity: %1%")
					.arg (info.Similarity_);
			else if (!info.SimilarTo_.isEmpty ())
				simStr = tr ("Similar to: %1")
					.arg (info.SimilarTo_.join ("; "));
			if (!simStr.isEmpty ())
				item->setData (simStr, SimilarModel::Role::Similarity);

			Model_->appendRow (item);
		}
	}

	void SimilarViewManager::handleSimilarReady ()
	{
		sender ()->deleteLater ();
		auto obj = qobject_cast<Media::IPendingSimilarArtists*> (sender ());

		const auto& similar = obj->GetSimilar ();
		SetInfos (similar);
	}
}
}
