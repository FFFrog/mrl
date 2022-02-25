# How to Debug Linux Kernel

## 编译内核

1. 下载kernel代码，切换至目标tag

    ```shell
    git clone git@github.com:FFFrog/linux.git
    git checkout v5.10
    ```

2. 开启内核参数

    ```shell
    make menuconfig
    ```

    ```text
    Kernel hacking  ---> 
        -*- Kernel debugging
        Compile-time checks and compiler options  --->
            [*] Compile the kernel with debug info
            [*] Provide GDB scripts for kernel debugging

    Device Drivers  --->
        Generic Driver Options  --->
            [*] Support for uevent helper

    Device Drivers  --->
        Generic Driver Options  --->
            (/sbin/hotplug) path to uevent helper

    Device Drivers  --->
        Character devices  --->
            <*> Virtio console

    Device Drivers  --->
        [*] Virtio drivers  --->
            <*>   Platform bus driver for memory mapped virtio devices

    Device Drivers  --->
        [*] Virtio drivers  --->
            <*>   PCI driver for virtio devices

    Device Drivers  --->
        [*] Block devices  --->
            <*>   Virtio block driver

    [*] Networking support  --->
        <*>   Plan 9 Resource Sharing Support (9P2000)  --->

    [*] Networking support  --->
        <*>   Plan 9 Resource Sharing Support (9P2000)  --->
            <*>   9P Virtio Transport

    File systems  --->
        [*] Network File Systems  --->
            <*>   Plan 9 Resource Sharing Support (9P2000)

    File systems  --->
        [*] Network File Systems  --->
            <*>   Plan 9 Resource Sharing Support (9P2000)
                [*]     9P POSIX Access Control Lists

    File systems  --->
        <*> The Extended 4 (ext4) filesystem
    ```

    配置完成后在当前目录生成 `.config` 文件

    ```shell
    grep -Ew "CONFIG_DEBUG_INFO|CONFIG_GDB_SCRIPTS|CONFIG_UEVENT_HELPER|CONFIG_UEVENT_HELPER_PATH|CONFIG_VIRTIO_CONSOLE|CONFIG_VIRTIO_MMIO|CONFIG_VIRTIO_PCI|CONFIG_VIRTIO|CONFIG_VIRTIO_BLK|CONFIG_NET_9P|CONFIG_NET_9P_VIRTIO|CONFIG_9P_FS|CONFIG_9P_FS_POSIX_ACL|CONFIG_EXT4_FS" .config
    ```

    ```text
    CONFIG_NET_9P=y
    CONFIG_NET_9P_VIRTIO=y
    CONFIG_UEVENT_HELPER=y
    CONFIG_UEVENT_HELPER_PATH="/sbin/hotplug"
    CONFIG_VIRTIO_BLK=y
    CONFIG_VIRTIO_CONSOLE=y
    CONFIG_VIRTIO=y
    CONFIG_VIRTIO_PCI=y
    CONFIG_VIRTIO_MMIO=y
    CONFIG_EXT4_FS=y
    CONFIG_9P_FS=y
    CONFIG_9P_FS_POSIX_ACL=y
    CONFIG_DEBUG_INFO=y
    CONFIG_GDB_SCRIPTS=y
    ```

3. 编译安装

    ```shell
    make -j 8
    ```

4. 内核镜像

    ```text
    # 未删除符号信息 && 未压缩内核文件（用于 gdb 读取 kernel 符号信息)
    vmlinux

    # 删除符号信息 && 未压缩内核文件
    arch/arm64/boot/Image

    # 删除符号信息 && 压缩内核文件
    arch/arm64/boot/Image.gz
    ```

## 制作rootfs

1. 下载toolbox代码，并解压

    ```shell
    wget http://busybox.net/downloads/busybox-1.34.0.tar.bz2
    tar -jxvf busybox-1.34.0.tar.bz2
    ```

2. 配置

    ```shell
    make menuconfig
    ```

    ```text
    Settings --->
    [*] Build static binary (no shared libs)  
    ```

3. 编译安装

    ```shell
    make -j 8
    make install
    ```

4. 完善目录结构

    ```shell
    cd _install

    mkdir -p lib lib64 home mnt
    mkdir -p proc var sys tmp run srv
    mkdir -p dev root etc/init.d

    mknod dev/console c 5 1
    mknod dev/null c 1 3
    mknod dev/tty1 c 4 1
    mknod dev/tty2 c 4 2
    mknod dev/tty3 c 4 3
    mknod dev/tty4 c 4 4

    cat << EOF > etc/profile
    #!/bin/sh
    export HOSTNAME=mrl
    export USER=root
    export HOME=/root
    export PS1="[$USER@$HOSTNAME \W]\# "
    PATH=/bin:/sbin:/usr/bin:/usr/sbin
    LD_LIBRARY_PATH=/lib:/usr/lib:/lib64:/usr/lib64:$LD_LIBRARY_PATH
    export PATH LD_LIBRARY_PATH
    EOF

    cat << EOF > etc/fstab
    #device  mount-point    type     options   dump   fsck order
    proc /proc proc defaults 0 0
    tmpfs /tmp tmpfs defaults 0 0
    sysfs /sys sysfs defaults 0 0
    tmpfs /dev tmpfs defaults 0 0
    debugfs /sys/kernel/debug debugfs defaults 0 0
    shared /mnt 9p trans=virtio 0 0
    EOF

    cat << EOF > etc/inittab
    ::sysinit:/etc/init.d/rcS
    ::respawn:-/bin/sh
    ::askfirst:-/bin/sh
    ::ctrlaltdel:/bin/umount -a -r
    EOF

    cat << EOF > etc/init.d/rcS
    /bin/mount -a
    mkdir -p /dev/pts
    mount -t devpts devpts /dev/pts
    echo /sbin/mdev > /proc/sys/kernel/hotplug
    mdev -s
    EOF

    chmod 755 etc/init.d/rcS
    ```

5. 制作虚拟磁盘文件

    ```shell
    mkdir loop

    dd if=/dev/zero of=rootfs.ext4.img bs=1M count=10240

    mkfs.ext4 rootfs.ext4
    mount -t ext4 -o loop rootfs.ext4 loop

    cp -af _install/* loop
    make modules_install INSTALL_MOD_PATH=/root/Git.d/c/linux/rootfs.ext4/

    umount loop

    rmdir loop
    ```

## 调试准备

1. 启动脚本 start.sh

    ```shell
    #!/bin/bash

    qemu-kvm -s \
             -S \
             -smp 2 \
             -cpu cortex-a57 \
             -m size=1024M \
             -nographic \
             -kernel linux/arch/arm64/boot/Image \
             -machine virt,virtualization=true,gic-version=3 \
             --fsdev local,id=shared,path=shared,security_model=none \
             -device virtio-9p-device,fsdev=shared,mount_tag=shared \
             -drive if=none,format=raw,file=rootfs.ext4,id=hd0 \
             -device virtio-blk-device,drive=hd0 \
             -netdev tap,id=nd0,ifname=tap0,script=no,downscript=no -device e1000,netdev=nd0 \
             --append "noinitrd root=/dev/vda rw nokaslr console=ttyAMA0 loglevel=8 rdinit=/linuxrc"
    ```

    - -s: 监听在 gdb 1234 端口
    - -S: 表示启动后就挂起，等待 gdb 连接；
    - -nographic: 不启动图形界面，调试信息输出到终端与参数 console=ttyS0 组合使用
    - -kernel: 指定启用的内核镜像
    - -append: 附加参数，其中 nokaslr 防止内核起始地址随机化导致 gdb 断点不能命中

2. 准备文件

    ```text
    /root
    └── Git.d
        └── c
            └── linux
                ├── start.sh
                ├── shared
                ├── rootfs.ext4
                ├── busybox-1.34.0
                └── .vscode
                    ├── c_cpp_properties.json
                    └── launch.json
                └── linux
                    ├── vmlinux
                    └── arch
                        └── arm64
                            └── boot
                                ├── Image
                                └── ..
    ```

3. 启动 qemu

    ```shell
    ./start.sh
    ```

4. 配置网络

    Host:

    ```shell
    brctl addbr kernel0
    brctl addif kernel0 tap0

    ip a add 10.0.0.1/24 dev kernel0
    ip link set dev tap0 up
    ip link set dev kernel0 up

    iptables -t nat -A POSTROUTING -s 10.0.0.0/24 ! -o kernel0 -j MASQUERADE
    ```

    Guest:

    ```shell
    modprobe e1000
    modprobe bridge

    ip a add 10.0.0.2/24 dev eth0
    ip link set dev eth0 up
    ip link set dev lo up
    ip route add default via 10.0.0.1 dev eth0
    ```

## gdb调试

gdb 命令行调试

```shell
gdb
(gdb) file vmlinux
(gdb) target remote :1234
(gdb) break start_kernel
(gdb) c
```

## vscode调试

1. 安装 VSCode Remote-SSH 插件

   详情参考 VSCode 说明

2. 配置 sshd 转发

    ```shell
    vim /etc/ssh/sshd_config

    AllowTcpForwarding yes
    ```

3. 配置 VSCode C/C++ 配置文件

    c_cpp_properties.json

    ```text
    {
        "version": 4,
        "configurations": [
            {
                "name": "Linux",
                "cStandard": "c11",
                "cppStandard": "c++11",
                "intelliSenseMode": "linux-gcc-arm64",
                "compilerPath": "/usr/bin/gcc",
                "defines": [
                    "_DEBUG",
                    "UNICODE"
                ],
                "includePath": [
                    "/usr/lib/gcc/aarch64-linux-gnu/7.3.0/include",
                    "/usr/local/include",
                    "/usr/include",
                    "${workspaceFolder}/linux/**"
                ],
                "browse": {
                    "path": [
                        "/usr/lib/gcc/aarch64-linux-gnu/7.3.0/include",
                        "/usr/local/include",
                        "/usr/include",
                        "${workspaceFolder}/linux"
                    ],
                    "limitSymbolsToIncludedHeaders": true
                }
            }
        ]
    }
    ```

    launch.json

    ```text
    {
        "version": "0.2.0",
        "configurations": [
            {
                "name": "C",
                "type": "cppdbg",
                "request": "launch",
                "args": [],
                "environment": [],
                "cwd": "${workspaceFolder}",
                "program": "${workspaceFolder}/linux/vmlinux",
                "MIMode": "gdb",
                "miDebuggerServerAddress": "127.0.0.1:1234",
                "targetArchitecture": "arm64",
                "stopAtEntry": false,
                "externalConsole": false,
                // sourceFileMap 此项请根据具体情况决定是否配置以及具体配置值
                "sourceFileMap": {
                    "/root/Git.d/c/linux": "/root/Git.d/c/linux/linux"
                }
            }
        ]
    }
    ```
