# Openstack Installation Guides

## Prerequisites in Control Node

```bash
yum install mariadb mariadb-server MySQL-python python2-PyMySQL -y

vim /etc/my.cnf.d/mariadb_openstack.cnf
--------------------------------------------------------------------
# 在[mysqld]添加以下配置
[mysqld]
bind-address = 0.0.0.0
default-storage-engine = innodb
innodb_file_per_table = on
max_connections = 4096
collation-server = utf8_general_ci
character-set-server = utf8
init-connect = 'SET NAMES utf8'
--------------------------------------------------------------------

systemctl enable mariadb.service
systemctl restart mariadb.service
systemctl status mariadb.service

/usr/bin/mysql_secure_installation -> Passwd:123456
systemctl restart mariadb.service
```

```bash
yum install rabbitmq-server -y

systemctl enable rabbitmq-server.service
systemctl start rabbitmq-server.service
systemctl status rabbitmq-server.service

rabbitmqctl add_user openstack openstack
rabbitmqctl set_permissions openstack ".*" ".*" ".*"
rabbitmqctl set_permissions -p "/" openstack ".*" ".*" ".*"

rabbitmq-plugins list

rabbitmq-plugins enable rabbitmq_management
systemctl restart rabbitmq-server.service
rabbitmq-plugins list
lsof -i:15672
```

```bash
yum install memcached python-memcached -y

vim /etc/sysconfig/memcached
--------------------------------------------------------------------
OPTIONS="-l 127.0.0.1,controller"
--------------------------------------------------------------------

systemctl enable memcached.service
systemctl start memcached.service
systemctl status memcached.service

```

```bash
yum install etcd -y

vim /etc/etcd/etcd.conf
--------------------------------------------------------------------
#[Member]
ETCD_DATA_DIR="/var/lib/etcd/default.etcd"
ETCD_LISTEN_PEER_URLS="http://192.168.0.200:2380"
ETCD_LISTEN_CLIENT_URLS="http://192.168.0.200:2379"
ETCD_NAME="controller"

#[Clustering]
ETCD_INITIAL_ADVERTISE_PEER_URLS="http://192.168.0.200:2380"
ETCD_ADVERTISE_CLIENT_URLS="http://192.168.0.200:2379"
ETCD_INITIAL_CLUSTER="controller=http://192.168.0.200:2380"
ETCD_INITIAL_CLUSTER_TOKEN="etcd-cluster-01"
ETCD_INITIAL_CLUSTER_STATE="new"
--------------------------------------------------------------------

systemctl enable etcd.service
systemctl start etcd.service
systemctl status etcd.service
```

## Keystone in Control Node

```bash
mysql -p123456

CREATE DATABASE keystone;

GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'localhost' IDENTIFIED BY 'keystone';
GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'%' IDENTIFIED BY 'keystone';
```

```bash
yum install openstack-keystone httpd mod_wsgi

vim /etc/keystone/keystone.conf
--------------------------------------------------------------------
[database]
connection = mysql+pymysql://keystone:keystone@controller/keystone

[token]
provider = fernet
--------------------------------------------------------------------
```

```bash
su -s /bin/sh -c "keystone-manage db_sync" keystone

mysql -h192.168.0.200 -ukeystone -pkeystone -e "use keystone;show tables;"
```

```bash
keystone-manage fernet_setup --keystone-user keystone --keystone-group keystone
keystone-manage credential_setup --keystone-user keystone --keystone-group keystone

keystone-manage bootstrap --bootstrap-password 123456 \
  --bootstrap-admin-url http://controller:5000/v3/ \
  --bootstrap-internal-url http://controller:5000/v3/ \
  --bootstrap-public-url http://controller:5000/v3/ \
  --bootstrap-region-id RegionOne
```

```bash
vim /etc/httpd/conf/httpd.conf +95
--------------------------------------------------------------------
ServerName controller
--------------------------------------------------------------------

ln -s /usr/share/keystone/wsgi-keystone.conf /etc/httpd/conf.d/

systemctl enable httpd.service
systemctl start httpd.service
systemctl status httpd.service
```

```bash
export OS_PROJECT_DOMAIN_NAME=Default
export OS_PROJECT_NAME=admin
export OS_USER_DOMAIN_NAME=Default
export OS_USERNAME=admin
export OS_PASSWORD=123456
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3
```

```bash
cat << EOF >~/.admin-openrc
export OS_PROJECT_DOMAIN_NAME=Default
export OS_PROJECT_NAME=admin
export OS_USER_DOMAIN_NAME=Default
export OS_USERNAME=admin
export OS_PASSWORD=123456
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3
EOF
```

```bash
openstack endpoint list
openstack project list
openstack user list
```

## Glance in Control Node

```bash
mysql -p123456

CREATE DATABASE glance;

GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'localhost' IDENTIFIED BY 'glance';
GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'%' IDENTIFIED BY 'glance';

flush privileges;
```

```bash
source ~/.admin-openrc

openstack user create --domain default --password=glance glance
openstack user list

openstack role add --project service --user glance admin

openstack service create --name glance --description "OpenStack Image" image
openstack service list

openstack endpoint create --region RegionOne image public http://controller:9292
openstack endpoint create --region RegionOne image internal http://controller:9292
openstack endpoint create --region RegionOne image admin http://controller:9292
openstack endpoint list
```

```bash
vim /etc/glance/glance-api.conf
--------------------------------------------------------------------
[database]
connection = mysql+pymysql://glance:glance@controller/glance

[keystone_authtoken]
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = glance
password = glance

[paste_deploy]
flavor = keystone

[glance_store]
stores = file,http
default_store = file
filesystem_store_datadir = /var/lib/glance/images/
--------------------------------------------------------------------

vim /etc/glance/glance-registry.conf
--------------------------------------------------------------------
[database]
connection = mysql+pymysql://glance:glance@controller/glance

[keystone_authtoken]
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = glance
password = glance

[paste_deploy]
flavor = keystone
--------------------------------------------------------------------
```

```bash
su -s /bin/sh -c "glance-manage db_sync" glance

mysql -h192.168.0.200 -uglance -pglance -e "use glance;show tables;"
```

```bash
systemctl enable openstack-glance-api.service openstack-glance-registry.service
systemctl start openstack-glance-api.service openstack-glance-registry.service
systemctl status openstack-glance-api.service openstack-glance-registry.service
```

```bash
wget --no-check-certificate http://download.cirros-cloud.net/0.3.5/cirros-0.3.5-x86_64-disk.img

openstack image create "cirros" --file cirros-0.3.5-x86_64-disk.img --disk-format qcow2 --container-format bare --public

openstack image list
```

## Nova in Control Node

```bash
mysql -u root -p123456

CREATE DATABASE nova_api;
CREATE DATABASE nova;
CREATE DATABASE nova_cell0;
CREATE DATABASE placement;

GRANT ALL PRIVILEGES ON nova_api.* TO 'nova'@'localhost' IDENTIFIED BY 'nova';
GRANT ALL PRIVILEGES ON nova_api.* TO 'nova'@'%' IDENTIFIED BY 'nova';

GRANT ALL PRIVILEGES ON nova.* TO 'nova'@'localhost' IDENTIFIED BY 'nova';
GRANT ALL PRIVILEGES ON nova.* TO 'nova'@'%' IDENTIFIED BY 'nova';

GRANT ALL PRIVILEGES ON nova_cell0.* TO 'nova'@'localhost' IDENTIFIED BY 'nova';
GRANT ALL PRIVILEGES ON nova_cell0.* TO 'nova'@'%' IDENTIFIED BY 'nova';

GRANT ALL PRIVILEGES ON placement.* TO 'placement'@'localhost' IDENTIFIED BY 'placement';
GRANT ALL PRIVILEGES ON placement.* TO 'placement'@'%' IDENTIFIED BY 'placement';

flush privileges;
```

```bash

source ~/.admin-openrc

openstack user create --domain default --password=nova nova
openstack user list

openstack role add --project service --user nova admin
openstack role list

openstack service create --name nova --description "OpenStack Compute" compute
openstack service list

openstack endpoint create --region RegionOne compute public http://controller:8774/v2.1
openstack endpoint create --region RegionOne compute internal http://controller:8774/v2.1
openstack endpoint create --region RegionOne compute admin http://controller:8774/v2.1
openstack endpoint list

openstack user create --domain default --password=placement placement
openstack user list

openstack role add --project service --user placement admin
openstack role list

openstack service create --name placement --description "Placement API" placement
openstack service list

openstack endpoint create --region RegionOne placement public http://controller:8778
openstack endpoint create --region RegionOne placement internal http://controller:8778
openstack endpoint create --region RegionOne placement admin http://controller:8778
openstack endpoint list
```

```bash
yum install openstack-nova-api openstack-nova-conductor \
            openstack-nova-console openstack-nova-novncproxy \
            openstack-nova-scheduler openstack-nova-placement-api -y
```

```bash
openstack-config --set  /etc/nova/nova.conf DEFAULT enabled_apis  osapi_compute,metadata
openstack-config --set  /etc/nova/nova.conf DEFAULT my_ip 192.168.0.200
openstack-config --set  /etc/nova/nova.conf DEFAULT use_neutron  true
openstack-config --set  /etc/nova/nova.conf DEFAULT firewall_driver  nova.virt.firewall.NoopFirewallDriver
openstack-config --set  /etc/nova/nova.conf DEFAULT transport_url  rabbit://openstack:openstack@controller
openstack-config --set  /etc/nova/nova.conf api_database connection  mysql+pymysql://nova:nova@controller/nova_api
openstack-config --set  /etc/nova/nova.conf database connection  mysql+pymysql://nova:nova@controller/nova
openstack-config --set  /etc/nova/nova.conf placement_database connection  mysql+pymysql://placement:placement@controller/placement
openstack-config --set  /etc/nova/nova.conf api auth_strategy  keystone
openstack-config --set  /etc/nova/nova.conf keystone_authtoken auth_url  http://controller:5000/v3
openstack-config --set  /etc/nova/nova.conf keystone_authtoken memcached_servers  controller:11211
openstack-config --set  /etc/nova/nova.conf keystone_authtoken auth_type  password
openstack-config --set  /etc/nova/nova.conf keystone_authtoken project_domain_name  default
openstack-config --set  /etc/nova/nova.conf keystone_authtoken user_domain_name  default
openstack-config --set  /etc/nova/nova.conf keystone_authtoken project_name  service
openstack-config --set  /etc/nova/nova.conf keystone_authtoken username  nova
openstack-config --set  /etc/nova/nova.conf keystone_authtoken password  nova
openstack-config --set  /etc/nova/nova.conf vnc enabled true
openstack-config --set  /etc/nova/nova.conf vnc server_listen '$my_ip'
openstack-config --set  /etc/nova/nova.conf vnc server_proxyclient_address '$my_ip'
openstack-config --set  /etc/nova/nova.conf glance api_servers  http://controller:9292
openstack-config --set  /etc/nova/nova.conf oslo_concurrency lock_path  /var/lib/nova/tmp
openstack-config --set  /etc/nova/nova.conf placement region_name RegionOne
openstack-config --set  /etc/nova/nova.conf placement project_domain_name Default
openstack-config --set  /etc/nova/nova.conf placement project_name service
openstack-config --set  /etc/nova/nova.conf placement auth_type password
openstack-config --set  /etc/nova/nova.conf placement user_domain_name Default
openstack-config --set  /etc/nova/nova.conf placement auth_url http://controller:5000/v3
openstack-config --set  /etc/nova/nova.conf placement username placement
openstack-config --set  /etc/nova/nova.conf placement password placement
openstack-config --set  /etc/nova/nova.conf scheduler discover_hosts_in_cells_interval 300

vim /etc/httpd/conf.d/00-nova-placement-api.conf
--------------------------------------------------------------------
<Directory /usr/bin>
   <IfVersion >= 2.4>
      Require all granted
   </IfVersion>
   <IfVersion < 2.4>
      Order allow,deny
      Allow from all
   </IfVersion>
</Directory>
--------------------------------------------------------------------

systemctl restart httpd
```

```bash
su -s /bin/sh -c "nova-manage api_db sync" nova

mysql -h192.168.0.200 -unova -pnova -e "use nova_api;show tables;"
mysql -h192.168.0.200 -uplacement -pplacement -e "use placement;show tables;"

su -s /bin/sh -c "nova-manage cell_v2 map_cell0" nova
su -s /bin/sh -c "nova-manage cell_v2 create_cell --name=cell1 --verbose" nova
su -s /bin/sh -c "nova-manage db sync" nova

mysql -h192.168.0.200 -unova -pnova -e "use nova_cell0;show tables;"
mysql -h192.168.0.200 -unova -pnova -e "use nova;show tables;"

su -s /bin/sh -c "nova-manage cell_v2 list_cells" nova
```

```bash
systemctl enable openstack-nova-api.service \
                 openstack-nova-consoleauth openstack-nova-scheduler.service \
                 openstack-nova-conductor.service openstack-nova-novncproxy.service
systemctl start openstack-nova-api.service \
                openstack-nova-consoleauth openstack-nova-scheduler.service \
                openstack-nova-conductor.service openstack-nova-novncproxy.service
```

## Nova in Compute Node

```bash
yum install openstack-nova-compute -y
```

```bash
openstack-config --set  /etc/nova/nova.conf DEFAULT my_ip 192.168.0.201
openstack-config --set  /etc/nova/nova.conf DEFAULT use_neutron True
openstack-config --set  /etc/nova/nova.conf DEFAULT firewall_driver nova.virt.firewall.NoopFirewallDriver
openstack-config --set  /etc/nova/nova.conf DEFAULT enabled_apis  osapi_compute,metadata
openstack-config --set  /etc/nova/nova.conf DEFAULT transport_url  rabbit://openstack:openstack@controller
openstack-config --set  /etc/nova/nova.conf api auth_strategy  keystone
openstack-config --set  /etc/nova/nova.conf keystone_authtoken auth_url http://controller:5000/v3
openstack-config --set  /etc/nova/nova.conf keystone_authtoken memcached_servers controller:11211
openstack-config --set  /etc/nova/nova.conf keystone_authtoken auth_type password
openstack-config --set  /etc/nova/nova.conf keystone_authtoken project_domain_name default
openstack-config --set  /etc/nova/nova.conf keystone_authtoken user_domain_name default
openstack-config --set  /etc/nova/nova.conf keystone_authtoken project_name  service
openstack-config --set  /etc/nova/nova.conf keystone_authtoken username nova
openstack-config --set  /etc/nova/nova.conf keystone_authtoken password nova
openstack-config --set  /etc/nova/nova.conf vnc enabled True
openstack-config --set  /etc/nova/nova.conf vnc server_listen 0.0.0.0
openstack-config --set  /etc/nova/nova.conf vnc server_proxyclient_address  '$my_ip'
openstack-config --set  /etc/nova/nova.conf vnc novncproxy_base_url  http://controller:6080/vnc_auto.html
openstack-config --set  /etc/nova/nova.conf glance api_servers http://controller:9292
openstack-config --set  /etc/nova/nova.conf oslo_concurrency lock_path /var/lib/nova/tmp
openstack-config --set  /etc/nova/nova.conf placement region_name RegionOne
openstack-config --set  /etc/nova/nova.conf placement project_domain_name Default
openstack-config --set  /etc/nova/nova.conf placement project_name service
openstack-config --set  /etc/nova/nova.conf placement auth_type password
openstack-config --set  /etc/nova/nova.conf placement user_domain_name Default
openstack-config --set  /etc/nova/nova.conf placement auth_url http://controller:5000/v3
openstack-config --set  /etc/nova/nova.conf placement username placement
openstack-config --set  /etc/nova/nova.conf placement password placement

egrep -c '(vmx|svm)' /proc/cpuinfo
openstack-config --set  /etc/nova/nova.conf libvirt virt_type  qemu
egrep -v "^#|^$" /etc/nova/nova.conf|grep 'virt_type'
```

```bash
systemctl enable libvirtd.service openstack-nova-compute.service
systemctl start libvirtd.service openstack-nova-compute.service
```

## Nova in Compute Node(Append)

> TODO

```bash
source ~/.admin-openrc

openstack compute service list --service nova-compute
su -s /bin/sh -c "nova-manage cell_v2 discover_hosts --verbose" nova
```

```bash
source ~/.admin-openrc

openstack compute service list
openstack catalog list
openstack image list
nova-status upgrade check
```
