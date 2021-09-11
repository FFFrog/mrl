# Openstack Command Line Manual

<!-- TOC -->

- [Openstack Command Line Manual](#openstack-command-line-manual)
  - [glance命令](#glance命令)
    - [修改qcow2镜像密码](#修改qcow2镜像密码)
    - [创建镜像](#创建镜像)
  - [nova命令](#nova命令)
    - [创建flavor](#创建flavor)
    - [创建虚拟机](#创建虚拟机)
  - [neutron命令](#neutron命令)
  - [创建网络](#创建网络)
    - [创建租户网络](#创建租户网络)
    - [创建外部网络](#创建外部网络)
    - [创建路由](#创建路由)
  - [OVS命令](#ovs命令)
    - [ovs-vsctl](#ovs-vsctl)
    - [ovs-appctl](#ovs-appctl)
    - [ovs-ofctl](#ovs-ofctl)

<!-- /TOC -->

## glance命令

### 修改qcow2镜像密码

安装工具包

```shell
yum -y install libguestfs-tools
```

生成密码秘闻

```shell
openssl passwd -1 centos
$1$Tu/FNjfV$VdglOOIbZIRhzDWdD5H4r.
```

修改镜像密码，二选一

```shell
guestfish --rw -a CentOS-7-aarch64-GenericCloud-2003.qcow2c
<fs> run
<fs> list-filesystems
<fs> mount /dev/sda1 /
<fs> vi /etc/shadow
<fs> quit
```

```shell
virt-customize -a CentOS-7-aarch64-GenericCloud-2003.qcow2c \
--root-password password:$1$Tu/FNjfV$VdglOOIbZIRhzDWdD5H4r.
[   0.0] Examining the guest ...
[   1.9] Setting a random seed
[   1.9] Setting passwords
[   6.8] Finishing off
```

### 创建镜像

```shell
openstack image create --file cirros-0.4.0-aarch64-disk.img --disk-format qcow2 --container-format bare --public cirros
openstack image create --file CentOS-7-aarch64-GenericCloud-2003.qcow2c --disk-format qcow2 --container-format bare --public centos
```

## nova命令

### 创建flavor

```shell
openstack flavor create --id small --ram 1024 --disk 10 --vcpus 1 --public small
openstack flavor create --id middle --ram 2048 --disk 20 --vcpus 2 --public middle
openstack flavor create --id large --ram 4096 --disk 50 --vcpus 4 --public large
```

### 创建虚拟机

```shell
openstack server create --image cirros --flavor small --nic net-id=mrl cirros
openstack server create --image centos --flavor large --nic net-id=mrl centos
```

## neutron命令

## 创建网络

### 创建租户网络

```shell
openstack network create mrl --internal
openstack subnet create --network mrl --gateway 192.168.188.1 --subnet-range 192.168.188.0/24 mrl_subnet
```

### 创建外部网络

```shell
openstack network create --external --share --provider-network-type flat \
                                    --provider-physical-network provider mrl_external
openstack subnet create --no-dhcp --subnet-range 192.168.1.0/24 --gateway 192.168.1.1 --ip-version 4 \
                        --network mrl_external --allocation-pool start=192.168.1.211,end=192.168.1.211 \
                        --dns-nameserver 114.114.115.115 mrl_external_subnet
```

### 创建路由

```shell
openstack router create mrl
openstack router set --external-gateway mrl_external mrl
neutron router-interface-add ROUTER SUBNET
```

## OVS命令

### ovs-vsctl

ovs vswitch信息

```shell
ovs-vsctl show
```

### ovs-appctl

ovs 流表抓包

```shell
ovs-appctl ofproto/trace br-int in_port=37
```

ovs mac表

```shell
ovs-appctl fdb/show br-int
```

### ovs-ofctl

ovs dump流表

```shell
ovs-ofctl dump-flows br-int
```
