# cpio手册

## 基本介绍

1. Copy-out模式：此模式下，cpio将向归档文件中拷入文件，即进行归档操作，所以成为归档模式。它会从标准输入中读取待归档的文件，将它们归档到目标目标中，若未指定归档的目标，将归档到标准输出中。在copy-out模式下，最典型的是使用find来指定待归档文件，在使用find时，最好加上"-depth"以尽可能减少可能出现的问题，例如目录的权限问题。
2. Copy-in模式：此模式下，cpio将从归档文件中提取文件，或者列出归档文件中的文件列表。它将从标准输入中读取归档文件。任意cpio的非选项参数都将认为是shell的glob通配pattern，只有文件名匹配了指定模式时才会从中提取出来或list出来。在cpio中，通配符不能匹配到"."或"/"，所以如有需要，必须显式指定"."或"/"。如果没有指定匹配模式，则解压或列出所有文件。
3. Copy-pass模式：此模式下，cpio拷贝一个目录树(即目录中所有文件)到另一个目录下，并在目标目录下以同名的子目录存在。copy-pass模式是copy-in模式再copy-out模式的结合，它中途没有涉及到任何归档行为。这是cpio的一个特殊用法。它从标准输入中读取待拷贝内容，然后将它们复制到目标路径下。

## 参数

对于cpio这个归档工具来说，它和其它命令有一个区别：在cpio命令行中给出的非选项参数都会认为是pattern。非选项参数的意思是这个参数不是为选项指定的参数。如cpio -t abc < a.cpio中，"-t"选项的功能是列出文件列表，它不需要参数，但后面给定了abc，则表示列出匹配abc的文件或目录。

选项说明：

```text
-o：(--create)指定运行为copy-out模式，即归档模式
-i：(--extract)指定运行为copy-in模式，即提取模式
-p：(--pass-through)指定运行为copy-pass模式，即目录拷贝模式
-t：(--list)列出归档文件中的文件列表

-B：设置I/O的block大小为5120字节，默认为512字节
-C IO-SIZE：(--io-size=IO-SIZE)指定I/O的block大小为IO-SIZE大小

-I archive：使用指定的归档文件名替代标准输入(从标准输入读取的，所以肯定是解压模式，即copy-in)
-O archive：使用指定的归档文件名替代标准输出(输出到标准输出的，所以肯定是归档模式，即copy-out)
-F archive：(--file=archive)使用指定的归档文件名替代标准输入或输出。所以无论是copy-in还是copy-out模式都可以使用-F指定归档文件
          ：注意copy-out即归档模式下，其默认行为等价于重定向符号">"，所以内容会完全覆盖，但归档文件(inode)不变

-A：(--append)向已存在的归档文件中追加文件，只能使用-F或-O指定归档文件，只能用在copy-out模式下。不等价于重定向符号">>"，">>"后在归档文件中找不到追加的文件
--to-stdout：解压文件到标准输出，用在copy-in模式

-E FILE：(--pattern-file=FILE)从FILE中读取pattern进行匹配，匹配到的将解压或列出它们，用在copy-in模式。
-f：(--nonmatching)仅copy不匹配指定pattern的文件

-u：(--unconditional)当目标中有同名文件时，强制替换冲突文件
-a：(--reset-access-time)重置文件的atime，即保留文件的原始atime
-m：(--preserve-modification-time)保留文件的原始mtime
-d：(-make-directories)当需要的时候自动创建目录

-0：(--null)解析空字符串\0
--quiet：不输出拷贝时的block数量信息
-v：给出详细信息
```

## 示例

1. 将家目录下的所有文件都归档到tree.cpio中。

    ```shell
    [root@server2 ~]# find ~ -depth | cpio -ov > tree.cpio 或 [root@server2 ~]# find ~ -depth | cpio -ov -F tree.cpio
    ```

    ***注意，如果使用find搜索，且归档文件和搜索目录是同一路径时，它会将归档文件本身也归档到归档文件中，即进行了迭代归档。***
    例如上面的例子中，将find家目录的所有文件都归档到tree.cpio，但tree.cpio也将是放在家目录下的，它也会被find搜素到，所以也会被归档到其自身中去。可能这里会有所疑惑了，不是应该find处理完之后才处理cpio吗？非也，管道的作用只是进程间数据传递的作用，但不是一定要等管道左边的处理完成之后才传递，而是左边边处理边传递到右边的，如果左边处理的太快，导致管道拥堵，那么管道左边的程序将等待管道有空闲空间。
    要解决迭代归档的问题，只要让归档文件不被find搜索到即可。可以在find中排除、在cpio中排除或归档文件放到其他目录下去。

    ```shell
    [root@server2 ~]# find ~ -depth | cpio -ov -F /tmp/tree.cpio
    ```

    一般出于准确性考虑，会在find上使用"-print0"，然后在cpio上使用"--null"解析空字符。

    ```shell
    [root@server2 ~]# find ~ -depth -print0 | cpio --null -ov -F /tmp/tree.cpio
    ```

2. 列出归档文件中的文件列表。

    ```shell
    [root@server2 tmp]# cpio -t -F tree.cpio
    ```

    或

    ```shell
    [root@server2 tmp]# cpio -t < tree.cpio
    ```

    这将会递归列出tree.cpio中的所有文件，所以这不会是你想要的，应该对其指定一个匹配模式。

    ```shell
    [root@server2 tmp]# cpio -t -F tree.cpio /root/*
    ```

    或

    ```shell
    [root@server2 tmp]# cpio -t /root/* < tree.cpio
    ```

    这将列出tree.cpio中所有/root/目录下的内容，但是很显然，"*"号没法匹配点开头的隐藏文件，所以不会显示出隐藏文件。

    ```shell
    [root@server2 tmp]# cpio -t -F tree.cpio /root/.*
    ```

    但这样又只会列出隐藏文件。如何既列出隐藏文件，也列出普通文件？在cpio中好像不直接支持这样的通配，以下是我想到的一个办法。

    ```shell
    [root@server2 tmp]# cpio -t -F tree.cpio /root/{.*,*}
    ```

    ***注意，cpio命令行中任意非选项参数都会被当成pattern。***

3. 向归档文件中追加文件。

    ***注意，不要使用重定向符号">>"进行追加，虽然从归档文件最后的打下上看是追加成功了，但是实际上你却不知道它追加到哪里去了，根本就找不到追加的内容。所以，还是用"-A"选项。***

    ```shell
    [root@server2 tmp]# ls /root/new.txt | cpio -oA -F tree.cpio
    ```

    这将会把/root/new.txt下的文件追加到tree.cpio中，由于tree.cpio中已经有了/root目录，所以追加后路径为tree.cpio中的/root/new.txt。而如果tree.cpio中没有待追加文件所在的目录，则会新建一个目录。

    ```shell
    [root@server2 tmp]# find /boot -depth -print0 | cpio -oA -F tree.cpio
    ```

    这将会在tree.cpio中追加一个/boot目录，它和/root目录是同级别的。

4. 提取文件。

    ***注意：cpio只能提取文件时只能提取到当前目录下。***

    ```shell
    [root@server2 tmp]# cpio -idv -F tree.cpio /root/
    new.txt
    cpio: new.txt not created: newer or same age version exists
    ```

    这就会提取/root/下的new.txt到当前目录，显然，这里报了一个错，提示当前目录下已经存在较待提取文件更新的同名文件，所以并没有进行提取。如果要强行提取，使用功能"-u"选项，这将覆盖已存在动作。

    ```shell
    [root@server2 tmp]# cpio -idvu -F tree.cpio new.txt
    ```

    其实，只要是同名文件，不管它的时间戳是否比待提取更新，都不会提取，除非强制提取。
    这里"-d"选项的作用是提取时如果前导目录不存在，则自动创建。反正不会影响已存在目录，所以"-i"一般都会加上个"-d"。

5. 目录文件复制，即copy-pass模式。

    ***注意，该模式下复制的目录在目标位置上是以子目录形式存在的。***
    例如复制/root目录到/tmp/abc下，则在/tmp/abc下会有root子目录，在/tmp/abc/root下才是源/root中的文件。

    ```shell
    [root@server2 tmp]# find ~ -depth -print0 | cpio --null -pvd /tmp/abc
    [root@server2 tmp]# ll /tmp/abc
    dr-xr-x--- 6 root root 4096 Jun 13 09:45 root
    ```
