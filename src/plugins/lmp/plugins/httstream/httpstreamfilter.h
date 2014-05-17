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

#pragma once

#include <QObject>
#include "interfaces/lmp/ifilterelement.h"
#include "interfaces/lmp/isourceobject.h"

typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;
typedef struct _GstPadTemplate GstPadTemplate;

namespace LeechCraft
{
namespace LMP
{
namespace HttStream
{
	class HttpServer;
	class FilterConfigurator;

	class HttpStreamFilter : public QObject
						   , public IFilterElement
	{
		Q_OBJECT

		const QByteArray FilterId_;
		const QByteArray InstanceId_;
		IPath * const Path_;

		FilterConfigurator * const Configurator_;

		GstElement * const Elem_;

		GstElement * const Tee_;

		GstPadTemplate * const TeeTemplate_;

		GstElement * const AudioQueue_;
		GstElement * const StreamQueue_;

		GstElement * const Encoder_;

		GstElement * const Muxer_;

		GstElement * const MSS_;

		HttpServer * const Server_;

		GstPad *TeeAudioPad_;
		GstPad *TeeStreamPad_ = nullptr;

		int ClientsCount_ = 0;

		SourceState StateOnFirst_ = SourceState::Error;
	public:
		HttpStreamFilter (const QByteArray& filterId,
				const QByteArray& instanceId, IPath *path);
		~HttpStreamFilter ();

		QByteArray GetEffectId () const;
		QByteArray GetInstanceId () const;
		IFilterConfigurator* GetConfigurator () const;

		void SetQuality (double);
		void SetAddress (const QString&, int);

		void HandleRemoved (int, int);
	protected:
		GstElement* GetElement () const;
		void PostAdd (IPath*) override;
	private:
		void CreatePad ();
		void DestroyPad ();

		void HandleFirstClientConnected ();
		void HandleLastClientDisconnected ();

		int HandleError (GstMessage*);
	private slots:
		void checkCreatePad (SourceState);

		void readdFd (int);

		void handleClient (int);
		void handleClientDisconnected (int);
	};
}
}
}
