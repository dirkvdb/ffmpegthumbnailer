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

#include <limits>

#include <QImage>
#include <QCheckBox>
#include <QFormLayout>
#include <QRegExpValidator>
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
    thumbCache.setMaxCost(KFFMpegThumbnailerSettings::cacheSize());
}

KFFMpegThumbnailer::~KFFMpegThumbnailer()
{
}

bool KFFMpegThumbnailer::create(const QString& path, int width, int /*height*/, QImage& img)
{
    int seqIdx = static_cast<int>(sequenceIndex());

    const QList<int> seekPercentages = KFFMpegThumbnailerSettings::sequenceSeekPercentages();
    const int numSeekPercentages = seekPercentages.size();

    seqIdx %= numSeekPercentages;

    const QString cacheKey = QString("%1$%2@%3").arg(path).arg(seqIdx).arg(width);

    QImage* cachedImg = thumbCache[cacheKey];
    if (cachedImg) {
        img = *cachedImg;
        return true;
    }

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
        m_Thumbnailer.setSeekPercentage(seekPercentages[seqIdx]);

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

    const int cacheCost = static_cast<int>((img.sizeInBytes()+1023) / 1024);
    thumbCache.insert(cacheKey, new QImage(img), cacheCost);

    return true;
}

float KFFMpegThumbnailer::sequenceIndexWraparoundPoint() const
{
    return static_cast<float>(KFFMpegThumbnailerSettings::sequenceSeekPercentages().size());
}

ThumbCreator::Flags KFFMpegThumbnailer::flags() const
{
    return static_cast<Flags>(None);
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

    QString seekPercentagesStr;
    for (const int sp : KFFMpegThumbnailerSettings::sequenceSeekPercentages()) {
        if (!seekPercentagesStr.isEmpty()) {
            seekPercentagesStr.append(' ');
        }
        seekPercentagesStr.append(QString().setNum(sp));
    }

    m_sequenceSeekPercentagesLineEdit = new QLineEdit();
    m_sequenceSeekPercentagesLineEdit->setText(seekPercentagesStr);
    formLayout->addRow("Sequence seek percentages", m_sequenceSeekPercentagesLineEdit);

    m_thumbCacheSizeSpinBox = new QSpinBox();
    m_thumbCacheSizeSpinBox->setRange(0, std::numeric_limits<int>::max());
    m_thumbCacheSizeSpinBox->setValue(KFFMpegThumbnailerSettings::cacheSize());
    formLayout->addRow("Cache size (KiB)", m_thumbCacheSizeSpinBox);

    return widget;
}

void KFFMpegThumbnailer::writeConfiguration(const QWidget* /*configurationWidget*/)
{
    KFFMpegThumbnailerSettings* settings = KFFMpegThumbnailerSettings::self();

    settings->setAddFilmstrip(m_addFilmStripCheckBox->isChecked());
    settings->setUseMetadataCovers(m_useMetadataCheckBox->isChecked());
    settings->setUseSmartSelection(m_useSmartSelectionCheckBox->isChecked());

    const QStringList seekPercentagesStrList = m_sequenceSeekPercentagesLineEdit->text()
            .split(QRegExp("(\\s*,\\s*)|\\s+"), Qt::SkipEmptyParts);
    QList<int> seekPercentages;
    bool seekPercentagesValid = true;

    for (const QString str : seekPercentagesStrList) {
        const int sp = str.toInt(&seekPercentagesValid);

        if (!seekPercentagesValid) {
            break;
        }

        seekPercentages << sp;
    }

    if (seekPercentagesValid) {
        settings->setSequenceSeekPercentages(seekPercentages);
    }

    settings->setCacheSize(m_thumbCacheSizeSpinBox->value());
    thumbCache.setMaxCost(m_thumbCacheSizeSpinBox->value());

    settings->save();
}
