# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=2
inherit multilib subversion

DESCRIPTION="Lightweight video thumbnailer that can be used by file managers"
HOMEPAGE="http://code.google.com/p/ffmpegthumbnailer"
ESVN_REPO_URI="http://ffmpegthumbnailer.googlecode.com/svn/trunk"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS=""
IUSE="png jpeg debug test"

RDEPEND="png? media-libs/libpng
	jpeg? media-libs/jpeg
	>=media-video/ffmpeg-0.4.9_p20070330
	test? dev-libs/gtest"

DEPEND="${RDEPEND}
	dev-util/pkgconfig"

src_configure() {
	econf \
		--disable-dependency-tracking \
		--disable-static \
		$(use_enable jpeg) \
		$(use_enable png) \
		$(use_enable debug) \
		$(use_enable test unittests)
}

src_install() {
	emake DESTDIR="${D}" install || die "emake install failed"
	dodoc AUTHORS ChangeLog README TODO
	rm -f "${D}"/usr/$(get_libdir)/libffmpegthumbnailer.la
}

