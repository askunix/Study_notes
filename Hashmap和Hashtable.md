map中HashMap和Hashtable主要有什么区别。
HashMap不是线程安全的，允许null key和null value。
Hashtable是Synchronize线程安全，不允许null key和null value

HashMap内部实际是采用了一种hash表的这种数据结构。
hash表我们又叫做散列表，hash表是根据关键码值(key value)而直接进行访问的数据结构。也就是说，它通过把关键码值映射到表中一个位置来访问记录。这个映射函数就是哈希函数，存放记录的数组就叫做hash表。
简单的说：hash表就是一整个数组与链表的集合。集成了数组遍历快和链表插入删除的优点。

---------------------

本文来自 Sydica 的CSDN 博客 ，全文地址请点击：https://blog.csdn.net/sydica/article/details/19621453?utm_source=copy 
