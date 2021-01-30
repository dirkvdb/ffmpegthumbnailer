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

#ifndef KFFMPEG_THUMBNAILER_H
#define KFFMPEG_THUMBNAILER_H

#include <QObject>
#include <QCache>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <kio/thumbsequencecreator.h>

#include <libffmpegthumbnailer/videothumbnailer.h>
#include <libffmpegthumbnailer/filmstripfilter.h>

class KFFMpegThumbnailer : public QObject, public ThumbSequenceCreator
{
    Q_OBJECT

private:
    typedef QCache<QString, QImage> ThumbCache;

public:
    KFFMpegThumbnailer();
    virtual ~KFFMpegThumbnailer();
    virtual bool create(const QString& path, int width, int height, QImage& img) override;
    virtual float sequenceIndexWraparoundPoint() const;
    virtual Flags flags() const override;
    virtual QWidget* createConfigurationWidget() override;
    virtual void writeConfiguration(const QWidget* configurationWidget) override;

private:
    ffmpegthumbnailer::VideoThumbnailer    m_Thumbnailer;
    ffmpegthumbnailer::FilmStripFilter     m_FilmStrip;
    ThumbCache thumbCache;
    QCheckBox*                             m_addFilmStripCheckBox;
    QCheckBox*                             m_useMetadataCheckBox;
    QCheckBox*                             m_useSmartSelectionCheckBox;
    QLineEdit*                             m_sequenceSeekPercentagesLineEdit;
    QSpinBox*                              m_thumbCacheSizeSpinBox;
};

#endif
