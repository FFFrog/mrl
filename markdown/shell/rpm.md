# rpm 手册

1. 查询rpm包中的脚本

    ```shell
    rpm -q --scripts openstack-glance
    ```

2. 查询rpm包的依赖

    ```shell
    rpm -q --requires
    ```

3. 查询文件属于哪个rpm包

    ```shell
    rpm -qf
    ```
