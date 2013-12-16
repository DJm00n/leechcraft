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

#pragma once

#include "utilconfig.h"
#include <QByteArray>
#include <QSet>
#include <QDataStream>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	/** @brief A simple pool of identificators of the given type.
	 *
	 * This class holds a pool of identificators of the given type \em T.
	 * It is very simple and produces consecutive IDs, this \em T should
	 * support <code>operator++()</code>.
	 */
	template<typename T>
	class IDPool
	{
		T CurrentID_;
	public:
		/** @brief Creates a pool with the given initial value.
		 *
		 * @param[in] id The initial value of the pool.
		 */
		IDPool (const T& id = T ())
		: CurrentID_ (id)
		{
		}

		/** @brief Destroys the pool.
		 */
		virtual ~IDPool ()
		{
		}

		/** @brief Returns next ID.
		 *
		 * @return Next ID in the pool.
		 */
		T GetID ()
		{
			return ++CurrentID_;
		}

		/** @brief Forcefully sets the current ID.
		 *
		 * @param[in] id The new current ID.
		 */
		void SetID (T id)
		{
			CurrentID_ = id;
		}

		/** @brief Frees the id.
		 *
		 * @param[in] id The ID to free.
		 */
		void FreeID (T id)
		{
			Q_UNUSED (id);
		}

		/** @brief Saves the state of this pool.
		 *
		 * @return The serialized state of this pool.
		 */
		QByteArray SaveState () const
		{
			QByteArray result;
			{
				QDataStream ostr (&result, QIODevice::WriteOnly);
				quint8 ver = 1;
				ostr << ver;
				ostr << CurrentID_;
			}
			return result;
		}

		/** @brief Recovers the state of this pool.
		 *
		 * @param[in] state The state of this pool obtained from
		 * SaveState().
		 */
		void LoadState (const QByteArray& state)
		{
			if (state.isEmpty ())
				return;

			QDataStream istr (state);
			quint8 ver;
			istr >> ver;
			if (ver == 1)
				istr >> CurrentID_;
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< ver
						<< ", not restoring state.";
		}
	};
}
}
