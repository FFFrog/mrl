# How to Debug Linux Kernel

## 编译内核

1. 下载kernel代码，切换至目标tag

    ```shell
    git clone git@github.com:FFFrog/linux.git
    git checkout v5.10
    ```

2. 开启内核参数 `CONFIG_DEBUG_INFO` 和 `CONFIG_GDB_SCRIPTS`。

    ```shell
    make menuconfig
    ```

    ```text
    Kernel hacking  ---> 
    [*] Kernel debugging
    Compile-time checks and compiler options  --->
        [*] Compile the kernel with debug info
        [*]   Provide GDB scripts for kernel debugging
    ```

    配置完成后在当前目录生成 `.config` 文件

    ```shell
    grep -E "CONFIG_DEBUG_INFO|CONFIG_GDB_SCRIPTS" .config
    ```

    ```text
    CONFIG_DEBUG_INFO=y
    CONFIG_GDB_SCRIPTS=y
    ```

3. 编译内核

    ```shell
    make -j 8
    ```

4. 内核镜像

    ```text
    # 未压缩的内核文件，在 gdb 的时候需要加载，用于读取 symbol 符号信息
    vmlinux

    # 删除 符号以及 debug 信息后的未压缩镜像
    arch/arm64/boot/Image

    # 删除 符号以及 debug 信息后的压缩镜像
    arch/arm64/boot/Image.gz
    ```

## 制作rootfs

1. 下载toolbox代码，并解压

    ```shell
    wget http://busybox.net/downloads/busybox-1.34.0.tar.bz2 --output busybox-1.34.0.tar.bz2
    tar -jxvf busybox-1.34.0.tar.bz2
    ```

2. 配置toolbox

    ```shell
    make menuconfif
    ```

    ```text
    Settings --->
    [*] Build static binary (no shared libs)  
    ```

3. 编译toolbox

    ```shell
    make -j 8
    make install
    ```

4. 完善目录结构

    ```shell
    cd _install

    mkdir -p proc sys dev tmp root etc/init.d

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
    LD_LIBRARY_PATH=/lib:/usr/lib:$LD_LIBRARY_PATH
    export PATH LD_LIBRARY_PATH
    EOF

    cat << EOF > etc/fstab
    #device  mount-point    type     options   dump   fsck order
    proc /proc proc defaults 0 0
    tmpfs /tmp tmpfs defaults 0 0
    sysfs /sys sysfs defaults 0 0
    tmpfs /dev tmpfs defaults 0 0
    debugfs /sys/kernel/debug debugfs defaults 0 0
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
    EOF

    chmod 755 etc/init.d/rcS

    find . | cpio -o -H newc | gzip > rootfs.img
    ```

## 调试kernel

1. qemu 启动 kernel

    ```shell
    qemu-kvm -s -S\
             -smp 2 \
             -cpu cortex-a57 \
             -m size=1024M \
             -nographic \
             -kernel Image \
             -initrd rootfs.img \
             -machine virt,virtualization=true,gic-version=3 \
             --append "nokaslr console=ttyAMA0 rdinit=/linuxrc"
    ```

    - -kernel Image: 指定启用的内核镜像
    - -initrd rootfs.img: 指定启动的内存文件系统
    - -append "nokaslr console=ttyS0": 附加参数，其中 nokaslr 防止内核起始地址随机化导致 gdb 断点不能命中
    - -s: 监听在 gdb 1234 端口
    - -S: 表示启动后就挂起，等待 gdb 连接；
    - -nographic: 不启动图形界面，调试信息输出到终端与参数 console=ttyS0 组合使用

2. gdb调试

    ```shell
    gdb 
    (gdb) file vmlinux
    (gdb) target remote :1234
    (gdb) break start_kernel
    (gdb) c 
    ```
