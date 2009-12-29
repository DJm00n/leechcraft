/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "wrapperobject.h"
#include <QIcon>
#include "coreproxywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			void* WrapperObject::qt_metacast (const char *_clname)
			{
				if (!_clname) return 0;
				if (!strcmp(_clname, "IInfo") &&
						Implements (_clname))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				if (!strcmp(_clname, "org.Deviant.LeechCraft.IInfo/1.0") &&
						Implements (_clname))
					return static_cast< IInfo*>(const_cast< WrapperObject*>(this));
				return QObject::qt_metacast(_clname);
			}

			WrapperObject::WrapperObject (const QString& filename,
					QObject *parent)
			: QObject (parent)
			, Filename_ (filename)
			{
				Module_ = PythonQt::self ()->createModuleFromFile (filename, filename);
			}

			void WrapperObject::Init (ICoreProxy_ptr proxy)
			{
				PythonQt::self ()->registerClass (&CoreProxyWrapper::staticMetaObject);
				QVariantList args;
				args << QVariant::fromValue<QObject*> (new CoreProxyWrapper (proxy));
				Call ("Init", args);
			}

			void WrapperObject::SecondInit ()
			{
				Call ("SecondInit");
			}

			void WrapperObject::Release ()
			{
				Call ("Release");
			}

			QString WrapperObject::GetName () const
			{
				return Call ("GetName").toString ();
			}

			QString WrapperObject::GetInfo () const
			{
				return Call ("GetInfo").toString ();
			}

			QIcon WrapperObject::GetIcon () const
			{
				return QIcon ();
			}

			QStringList WrapperObject::Provides () const
			{
				return Call ("Provides").toStringList ();
			}

			QStringList WrapperObject::Needs () const
			{
				return Call ("Needs").toStringList ();
			}

			QStringList WrapperObject::Uses () const
			{
				return Call ("Uses").toStringList ();
			}

			void WrapperObject::SetProvider (QObject*, const QString&)
			{
			}

			QVariant WrapperObject::Call (const QString& name, const QVariantList& args) const
			{
				return PythonQt::self ()->call (Module_, name, args);
			}

			bool WrapperObject::Implements (const char *interface)
			{
				return true;
			}
		};
	};
};

