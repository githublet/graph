# graph
Simple (directed) graph library.  NOTE: This library is building for gatk...

我在想能不能加一些中文。。。

那就简单描述下吧：首先点的id是从0递增的，所以设定了一个mod取余操作，作为一个点的hash值；

有了hash表，查找点的时间复杂度O(1)；删除点时会删除所有与这个点有关系的出边和入边，时间复杂度也从原来的O(点总数) 降低到 2倍点（边总数）；

设计hasCycle函数时，第一次设计思想是每一个点递归到底，撞库则环，发现太耗时了，还出现了死循环；
第二次设计加了栈，规避了死循环，加了is_visit,时间还是很多；
第三次设计加了is_exclude,时间复杂度完美了，最多O(点总数)；

开源是因为这个也是从GitHub上取到的代码，不过改的比较多，基本上重新设计了一遍。


