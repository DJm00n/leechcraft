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

#include "output.h"
#include <cmath>
#include <QtDebug>
#include <QTimer>
#include <gst/gst.h>
#include "path.h"
#include "../xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		/* Signature found on
		 * http://cgit.freedesktop.org/gstreamer/gst-plugins-base/commit/?id=de1db5ccbdc10a835a2dfdd5984892f3b0c9bcf4
		 *
		 * I love C language, it's freaking compile-time-safe and sane.
		 */
		gboolean CbVolumeChanged (GObject*, GParamSpec*, gpointer data)
		{
			auto output = static_cast<Output*> (data);
			const auto volume = output->GetVolume ();

			QMetaObject::invokeMethod (output,
					"volumeChanged",
					Q_ARG (qreal, volume));
			QMetaObject::invokeMethod (output,
					"volumeChanged",
					Q_ARG (int, volume * 100));

			return true;
		}

		gboolean CbMuteChanged (GObject*, GParamSpec*, gpointer data)
		{
			auto output = static_cast<Output*> (data);
			const auto isMuted = output->IsMuted ();

			QMetaObject::invokeMethod (output,
					"mutedChanged",
					Q_ARG (bool, isMuted));

			return true;
		}
	}

	Output::Output (QObject *parent)
	: QObject (parent)
	, Bin_ (gst_bin_new ("audio_sink_bin"))
	, Equalizer_ (gst_element_factory_make ("equalizer-3bands", "equalizer"))
	, Volume_ (gst_element_factory_make ("volume", "volume"))
	, Converter_ (gst_element_factory_make ("audioconvert", "convert"))
	, Sink_ (gst_element_factory_make ("autoaudiosink", "audio_sink"))
	, SaveVolumeScheduled_ (false)
	{
		gst_bin_add_many (GST_BIN (Bin_), Equalizer_, Volume_, Converter_, Sink_, nullptr);
		gst_element_link_many (Equalizer_, Volume_, Converter_, Sink_, nullptr);

		auto pad = gst_element_get_static_pad (Equalizer_, "sink");
		auto ghostPad = gst_ghost_pad_new ("sink", pad);
		gst_pad_set_active (ghostPad, TRUE);
		gst_element_add_pad (Bin_, ghostPad);
		gst_object_unref (pad);

		g_signal_connect (Volume_, "notify::volume", G_CALLBACK (CbVolumeChanged), this);
		g_signal_connect (Volume_, "notify::mute", G_CALLBACK (CbMuteChanged), this);

		const auto volume = XmlSettingsManager::Instance ()
				.Property ("AudioVolume", 1).toDouble ();
		setVolume (volume);

		const auto isMuted = XmlSettingsManager::Instance ()
				.Property ("AudioMuted", false).toBool ();
		g_object_set (G_OBJECT (Volume_), "mute", static_cast<gboolean> (isMuted), nullptr);
	}

	void Output::AddToPath (Path *path)
	{
		path->SetAudioBin (Bin_);
	}

	void Output::PostAdd (Path*)
	{
	}

	double Output::GetVolume () const
	{
		gdouble value = 1;
		g_object_get (G_OBJECT (Volume_), "volume", &value, nullptr);
		const auto exp = XmlSettingsManager::Instance ().property ("VolumeExponent").toDouble ();
		if (exp != 1)
			value = std::pow (value, 1 / exp);
		return value;
	}

	bool Output::IsMuted () const
	{
		gboolean value = false;
		g_object_get (G_OBJECT (Volume_), "mute", &value, nullptr);
		return value;
	}

	void Output::ScheduleSaveVolume ()
	{
		if (SaveVolumeScheduled_)
			return;

		SaveVolumeScheduled_ = true;
		QTimer::singleShot (1000,
				this,
				SLOT (saveVolume ()));
	}

	void Output::setVolume (double volume)
	{
		const auto exp = XmlSettingsManager::Instance ().property ("VolumeExponent").toDouble ();
		if (exp != 1)
			volume = std::pow (volume, exp);
		g_object_set (G_OBJECT (Volume_), "volume", static_cast<gdouble> (volume), nullptr);

		ScheduleSaveVolume ();
	}

	void Output::setVolume (int volume)
	{
		setVolume (volume / 100.);
	}

	void Output::toggleMuted ()
	{
		g_object_set (G_OBJECT (Volume_), "mute", static_cast<gboolean> (!IsMuted ()), nullptr);

		ScheduleSaveVolume ();
	}

	void Output::saveVolume ()
	{
		SaveVolumeScheduled_ = false;

		XmlSettingsManager::Instance ().setProperty ("AudioVolume", GetVolume ());
		XmlSettingsManager::Instance ().setProperty ("AudioMuted", IsMuted ());
	}
}
}
