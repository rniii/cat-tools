pkgname=cat-tools
pkgver=0.0
pkgrel=0
arch=(any)
depends=()
makedepends=(maid)
options=(!strip !debug)

build() {
  cd ..

  maid build
}

package() {
  cd ..

  dstdir="$pkgdir/usr" maid package
}
