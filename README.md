##Change List
基于axel-2.4源码做的开发，主要是修改了一些bug和添加了一些特性。   
>1. 添加-m选项，可以支持最大下载时间限定。
>2. 修改多源下载时的跳转bug。在多源下载时，如果存在30x跳转的情况，除第一个源以外的其他源上的下载全部失败。改进的方法是，在多线程启动前将存在跳转的源url替换成跳转后的url。
>3. 修改了gethostbyname在多进程下会出现异常的问题（使用可重入函数gethostbyname_r替换之)。
>4. 修改了下载过程中的进度输出方式，-a挺好用。
>5. 修改增加了对url中请求参数的支持。

##Build && Install
	./configure --prefix=/path/to/axel-install --i18n=0 --debug=1 --strip=0
	make && make install

