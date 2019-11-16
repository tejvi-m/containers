# usage : createFS <path_to_iso>
cd
mkdir temp
sudo mount -o loop $1 temp/
sudo unsquashfs -d rootfs/ temp/casper/filesystem.squashfs

#set up passwordless login by removing the x in the first line of rootfs/etc/passwd file
