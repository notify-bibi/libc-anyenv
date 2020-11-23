#!/bin/bash

target_path=$(cd "$(dirname "$0")"; pwd)
echo $target_path



die() {
  echo >&2 $1
  exit 1
}

usage() {
  echo >&2 "Usage: $0 id"
  exit 2
}


mk_dbg_info(){
   local debugpath=$1
   for file in `find "$debugpath/usr/lib/debug/lib" -name "*.so"`
   do
        local buildid=`readelf -n $file|grep 'Build ID'|awk '{print $3}'`
        local dir=`echo $buildid | cut -c1-2`
        local fn=`echo $buildid | cut -c3-`
        local target=${debugpath}/usr/lib/debug/.build-id/$dir
        local tdfile=$target/$fn

		echo "making dbg $file"
		mkdir -p $target
        ln -sf --relative $file $tdfile
        ln -sf --relative $file $tdfile.debug
   done
}

download_single() {
  local id=$1
  echo "Getting $id"
  if [ -d $target_path"/$id" ]; then
    mk_dbg_info $id
    die "  --> Downloaded before. Remove it to download again."
  fi

  if [ ! -f "db/$1.url" ]; then
    die "Invalid ID, maybe the library was fetched in an older version or added manually?"
  fi

  local url="$(cat "db/$1.url")"
  echo "  -> Location: $url"
  local tmp=${target_path}/${id}
  mkdir $tmp
  echo "  -> Downloading package to ${tmp}"
  wget "$url" 2>/dev/null -O $tmp/pkg.deb || die "Failed to download package from $url"

  local dbgurl=`echo $url | sed "s/libc6_/libc6-dbg_/"`
  echo "  -> Downloading package-dbg to ${tmp}"
  wget "$dbgurl" 2>/dev/null -O $tmp/pkg-dbg.deb || die "Failed to download package from $dbgurl"
  

  echo "  -> Extracting package"

  pushd $tmp 1>/dev/null
  ar x pkg.deb || die "ar failed"
  tar xf data.tar.* || die "tar failed"

  ar x pkg-dbg.deb || die "ar failed"
  tar xf data.tar.* || die "tar failed" 
  popd 1>/dev/null

  #mkdir libs/$id
  #cp $tmp/lib/*/* libs/$id 2>/dev/null || cp $tmp/lib32/* libs/$id 2>/dev/null \
  #  || die "Failed to save. Check it manually $tmp"
  echo "  -> Package saved to $id"

  #rm -rf $tmp


  mk_dbg_info $id
}

if [[ $# != 1 ]]; then
  usage
fi
download_single "$1"
