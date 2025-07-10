pkgname=wifi-manager
pkgver=1.0
pkgrel=1
pkgdesc="GTK interface for managing Wi-Fi connections via nmcli"
arch=('x86_64')
url="https://example.com"
license=('MIT')
depends=('gtk3' 'networkmanager')
makedepends=('gcc' 'pkgconf' 'gtk3')
source=('main.c' 'Makefile' 'wifi-manager.desktop' 'wifi-manager.png')
md5sums=('SKIP' 'SKIP' 'SKIP' 'SKIP')

build() {
  cd "$srcdir"
  make
}

package() {
  install -Dm755 "$srcdir/wifi-manager" "$pkgdir/usr/bin/wifi-manager"
  install -Dm644 "$srcdir/wifi-manager.desktop" "$pkgdir/usr/share/applications/wifi-manager.desktop"
  install -Dm644 "$srcdir/wifi-manager.png" "$pkgdir/usr/share/icons/hicolor/48x48/apps/wifi-manager.png"
}

