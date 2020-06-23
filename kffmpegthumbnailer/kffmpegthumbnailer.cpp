//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "kffmpegthumbnailer.h"
#include "kffmpegthumbnailersettings5.h"

#include <QImage>
#include <QCheckBox>
#include <QFormLayout>
#include <QWidget>

extern "C"
{
    Q_DECL_EXPORT ThumbCreator *new_creator()
    {
        return new KFFMpegThumbnailer();
    }
}


KFFMpegThumbnailer::KFFMpegThumbnailer()
{
}

KFFMpegThumbnailer::~KFFMpegThumbnailer()
{
}

bool KFFMpegThumbnailer::create(const QString& path, int width, int /*height*/, QImage& img)
{
    try
    {
        std::vector<uint8_t> pixelBuffer;
        KFFMpegThumbnailerSettings* settings = KFFMpegThumbnailerSettings::self();
        settings->load();

        if (settings->addFilmstrip())
        {
            m_Thumbnailer.addFilter(&m_FilmStrip);
        }
        else 
        {
            m_Thumbnailer.clearFilters();
        }

        m_Thumbnailer.setPreferEmbeddedMetadata(settings->useMetadataCovers());
        m_Thumbnailer.setSmartFrameSelection(settings->useSmartSelection());
        // 20% seek inside the video to generate the preview
        m_Thumbnailer.setSeekPercentage(20);

        m_Thumbnailer.setThumbnailSize(width);
        m_Thumbnailer.generateThumbnail(std::string(path.toUtf8()), Png, pixelBuffer);

        if (!img.loadFromData(&pixelBuffer.front(), pixelBuffer.size(), "PNG"))
        {
            return false;
        }
    }
    catch (std::exception& e)
    {
        return false;
    }

    return true;
}

ThumbCreator::Flags KFFMpegThumbnailer::flags() const
{
    return (Flags)(None);
}

QWidget *KFFMpegThumbnailer::createConfigurationWidget()
{
    QWidget* widget = new QWidget();
    QFormLayout* formLayout = new QFormLayout(widget);

    m_addFilmStripCheckBox = new QCheckBox("Embed filmstrip effect");
    m_addFilmStripCheckBox->setChecked(KFFMpegThumbnailerSettings::addFilmstrip());
    formLayout->addRow(m_addFilmStripCheckBox);

    m_useMetadataCheckBox = new QCheckBox("Use metadata embedded cover pictures");
    m_useMetadataCheckBox->setChecked(KFFMpegThumbnailerSettings::useMetadataCovers());
    formLayout->addRow(m_useMetadataCheckBox);

    m_useSmartSelectionCheckBox = new QCheckBox("Use smart (slower) frame selection");
    m_useSmartSelectionCheckBox->setChecked(KFFMpegThumbnailerSettings::useSmartSelection());
    formLayout->addRow(m_useSmartSelectionCheckBox);

    return widget;
}

void KFFMpegThumbnailer::writeConfiguration(const QWidget* /*configurationWidget*/)
{
    KFFMpegThumbnailerSettings* settings = KFFMpegThumbnailerSettings::self();

    settings->setAddFilmstrip(m_addFilmStripCheckBox->isChecked());
    settings->setUseMetadataCovers(m_useMetadataCheckBox->isChecked());
    settings->setUseSmartSelection(m_useSmartSelectionCheckBox->isChecked());

    settings->save();
}
