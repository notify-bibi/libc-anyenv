#!/bin/bash
user=chroot
group=gchroot

user_home="/home/${user}"
bin_root="/home/${user}/bin_root"



help(){
    exec 1>&2
    echo "--- help ---"
    echo "sudo ./mkenv.sh <libc6-id> <main_bin> [ dep_bins ... ]  -- Program execution in any environment"
    echo "sudo ./mkenv.sh uninstall   -- uninstall the change_root"
    echo "example: sudo ./mkenv.sh libc6_2.23-0ubuntu11.2_amd64 bash linux_server64 sh ls cat id"
    exit 1
}

[[ "$#" -le 1 ]] || [ "$1" == "-h" ] && help

if [[ " $@ " == *" uninstall "* ]] ; then
  umount ${bin_root}/proc
  read -r -p  "rm -rf ${bin_root} ? [Y/n]" sure
  if [[ ! "$sure" =~ ^(No|N|n)$ ]]
  then
    rm -rf ${bin_root}/
  fi
  exit
fi



# ------------------------- do it --------------------------


echo -e "user: $user:$group \nroot at $bin_root"

#create group if not exists
egrep "^$group" /etc/group >& /dev/null
if [ $? -ne 0 ]
then
    groupadd $group
fi

#create user if not exists
egrep "^$user" /etc/passwd >& /dev/null
if [ $? -ne 0 ]
then
    useradd -g $group -m -d $user_home $user 
fi




id=$1
die(){
   echo $@
   help
   exit 1
}

[[ ! -d "$id" ]] && die "not exist $id"

mkdir -p $bin_root

[[ ! -d "${bin_root}/proc" ]] && mkdir -p ${bin_root}/proc && mount --bind /proc ${bin_root}/proc
# [[ ! -d "${bin_root}/proc" ]] && mkdir ${bin_root}/proc && mount -t proc none ${bin_root}/proc

# [[ ! -d "${bin_root}/dev" ]] && mount --bind /dev ${bin_root}/dev

mkdir -p ${bin_root}/bin
mkdir -p ${bin_root}/dev
mknod -m 666 ${bin_root}/dev/null c 1 3
mknod -m 666 ${bin_root}/dev/zero c 1 5 
mknod -m 444 ${bin_root}/dev/random c 1 8
mknod -m 444 ${bin_root}/dev/urandom c 1 9 
mknod -m 666 ${bin_root}/dev/tty c 5 0 
mknod -m 666 ${bin_root}/dev/ptmx c 5 2 
mknod -m 622 ${bin_root}/dev/console c 5 1
    
cp -r $id/etc ${bin_root}/etc
cp -r $id/lib ${bin_root}/lib
cp -r $id/lib64 ${bin_root}/lib64
cp -r $id/usr ${bin_root}/usr



cmd=""
bin_fix_dep(){
    local arg=$1
    if [[ ! -f "$arg" ]] ; then
        arg=`which $arg`
		[[ ! -f "$arg" ]] && die "Cannot locate the $arg file"
    fi
    if [[ -z "$cmd" ]] ;then
        cmd=$arg
    fi
    local p=$(cd "$(dirname "$arg")"; pwd)
    echo -e "$arg \033[43;31;4m=>\033[0m ${bin_root}$arg"
    mkdir -p ${bin_root}$p
    cp $arg ${bin_root}$arg
    for deplib in $(ldd $arg | awk '{print $3}') ; do
        cp -r -f -n ${deplib} ${bin_root}${deplib}
        [[ ! -f "${bin_root}${deplib}" ]] && echo -e "  |_${deplib} \033[32m=>\033[0m bin_root${deplib}" 
        [[ -f "${bin_root}${deplib}" ]] && echo -e "  |_${deplib} \033[31;1m=X>\033[0m bin_root${deplib}"
        [[ ! -f "${bin_root}${deplib}" ]] && cp -n -f `readlink -f $deplib` bin_root`readlink -f $deplib`
    done
    
}  



for bin in ${@:2} ;do 
    bin_fix_dep $bin
done


echo -e "start \033[43;31;4m$cmd\033[0m "

chroot --userspec=${user}:${group} ${bin_root} $cmd


