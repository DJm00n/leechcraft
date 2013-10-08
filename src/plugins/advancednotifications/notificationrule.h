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

#ifndef PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULE_H
#define PLUGINS_ADVANCEDNOTIFICATIONS_NOTIFICATIONRULE_H
#include <QStringList>
#include <QMetaType>
#include "common.h"
#include "fieldmatch.h"

class QDataStream;

namespace LeechCraft
{
namespace AdvancedNotifications
{
	struct VisualParams
	{
	};

	bool operator== (const VisualParams&, const VisualParams&);

	struct AudioParams
	{
		QString Filename_;

		AudioParams ();
		AudioParams (const QString&);
	};

	bool operator== (const AudioParams&, const AudioParams&);

	struct TrayParams
	{
	};

	bool operator== (const TrayParams&, const TrayParams&);

	struct CmdParams
	{
		QString Cmd_;
		QStringList Args_;

		CmdParams ();
		CmdParams (const QString&, const QStringList& = QStringList ());
	};

	bool operator== (const CmdParams&, const CmdParams&);

	class NotificationRule
	{
		QString Name_;
		QString Category_;
		QStringList Types_;

		NotificationMethods Methods_;

		FieldMatches_t FieldMatches_;

		AudioParams AudioParams_;
		TrayParams TrayParams_;
		VisualParams VisualParams_;
		CmdParams CmdParams_;

		bool IsEnabled_;
		bool IsSingleShot_;
	public:
		NotificationRule ();
		NotificationRule (const QString& name,
				const QString& cat, const QStringList& types);

		bool IsNull () const;

		QString GetName () const;
		void SetName (const QString&);

		QString GetCategory () const;
		void SetCategory (const QString&);

		QSet<QString> GetTypes () const;
		void SetTypes (const QStringList&);

		NotificationMethods GetMethods () const;
		void SetMethods (const NotificationMethods&);
		void AddMethod (NotificationMethod);

		FieldMatches_t GetFieldMatches () const;
		void SetFieldMatches (const FieldMatches_t&);

		VisualParams GetVisualParams () const;
		void SetVisualParams (const VisualParams&);

		AudioParams GetAudioParams () const;
		void SetAudioParams (const AudioParams&);

		TrayParams GetTrayParams () const;
		void SetTrayParams (const TrayParams&);

		CmdParams GetCmdParams () const;
		void SetCmdParams (const CmdParams&);

		bool IsEnabled () const;
		void SetEnabled (bool);

		bool IsSingleShot () const;
		void SetSingleShot (bool);

		void Save (QDataStream&) const;
		void Load (QDataStream&);
	};

	bool operator== (const NotificationRule&, const NotificationRule&);
	bool operator!= (const NotificationRule&, const NotificationRule&);

	void DebugEquals (const NotificationRule&, const NotificationRule&);
}
}

Q_DECLARE_METATYPE (LeechCraft::AdvancedNotifications::NotificationRule);
Q_DECLARE_METATYPE (QList<LeechCraft::AdvancedNotifications::NotificationRule>);

#endif
